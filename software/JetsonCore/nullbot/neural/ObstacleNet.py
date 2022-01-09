from typing import Union
import torch, os
import pandas as pd
from PIL import Image
from torchvision.transforms import ToTensor
from torch import nn
from tqdm import tqdm

device = "cuda" if torch.cuda.is_available() else "cpu"
state_dict_name = "obstaclenet.pth"
model = None
optimizer = None
loss_fn = nn.CrossEntropyLoss()
to_tensor = ToTensor()
learning_rate = 1e-4
train_epochs = 4

def preprocess(img:Image) -> Image:
    img.thumbnail((400,400), Image.ANTIALIAS)
    return img

class ImageSet(torch.utils.data.Dataset):
    def __init__(self, img_dir, annotations_file, transform=None, target_transform=None, train=False):
        self.columns = ["file", "label"]
        self.img_dir = os.path.abspath(img_dir)
        self.df = pd.read_csv(os.path.join(self.img_dir, annotations_file), sep="\t", names=self.columns)
        self.transform = transform
        self.target_transform = target_transform
        

        #use first 80% for training and the rest 20% for testing 
        if( train ):
            self.df = self.df.iloc[:int(len(self.df)*0.8)]
            print("TRAIN LENGTH ", self.df.size)
            #self.df = self.df
        else:
            self.df = self.df.iloc[int(len(self.df)*0.8):]
            print("NOT TRAIN ", self.df.size)
            #self.df = self.df

    def __len__(self):
        return len(self.df)

    def __getitem__(self, idx):
        img_path = os.path.join(self.img_dir, self.df.iloc[idx, 0])
        img = Image.open(img_path)
        img = preprocess(img)

        label = self.df.iloc[idx, 1]

        if( self.transform ):
            img = self.transform(img)
        if( self.target_transform ):
            label = self.target_transform(label)

        return img, label
        
class Network(nn.Module):
    def __init__(self):
        super(Network, self).__init__()
        self.size_adapt = 15*50*94#15*50*94 test=15*69*94
        self.execution_stack = nn.Sequential(
            nn.Conv2d(3,10,kernel_size=4),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=4, stride=2),
            nn.Conv2d(10,15,kernel_size=8),
            nn.ReLU(),
            nn.MaxPool2d(kernel_size=4, stride=2),
        )
        self.connect = nn.Sequential(
            nn.Linear(self.size_adapt, 360),
            nn.ReLU(),
            nn.Linear(360, 160),
            nn.ReLU(),
            nn.Linear(160, 60),
            nn.ReLU(),
            nn.Linear(60, 2) #TODO change label output for more classification possibilities
        )

    def forward(self, x):
        y = self.execution_stack(x)
        #print(f"CONSTRUCT SUSPENSE TRIGGER [debug=>size {y.size()}]")
        #exit()
        y = y.view(-1, self.size_adapt)
        y = self.connect(y)
        return y

def get_model(pretrained=False) -> bool:
    global model
    if( pretrained ):
        try:
            model = torch.load(state_dict_name)
            model.eval()
            return True
        except:
            return False
    else:
        model = Network().to(device)
        return True

def save_model() -> None:
    torch.save(model, os.path.join(".", state_dict_name))

def __set_optimizer() -> None:
    global optimizer
    optimizer = torch.optim.Adam(model.parameters(), lr=learning_rate)

def train_network(img_path:str=None, annotations_file_name:str=None) -> None:
    if( model == None ):
        raise Exception("Cannot train without model")
    data_train = ImageSet(img_path, annotations_file_name, transform=to_tensor, train=True)
    data_test = ImageSet(img_path, annotations_file_name, transform=to_tensor, train=False)

    train_dataloader = torch.utils.data.DataLoader(data_train, batch_size=32, shuffle=True)
    test_dataloader = torch.utils.data.DataLoader(data_test, batch_size=32, shuffle=True)
    def train_loop() -> None:
        for X,y in tqdm(train_dataloader, desc="TRAINING"):
            X = X.to(device)
            y = y.to(device)

            optimizer.zero_grad()

            pred = model(X)
            loss = loss_fn(pred, y)

            loss.backward()
            optimizer.step()
        print(f"Result\nloss: {loss.item()}")

    def test_loop():
        size = len(test_dataloader.dataset)
        num_batches = len(test_dataloader)
        test_loss = 0
        correct = 0

        with torch.no_grad():
            for X,y in tqdm(test_dataloader, desc="TESTING"):
                X = X.to(device)
                y = y.to(device)

                pred = model(X)
                test_loss += loss_fn(pred, y).item()
                correct += (pred.argmax(1) == y).type(torch.float).sum().item()

        test_loss /= num_batches
        correct /= size
        print(f"Result\nAccuracy: {(100*correct):>0.1f}%, Avg loss: {test_loss:>8f}")
    
    print("TRAIN BEGIN")
    __set_optimizer()
    print("OPTIMIZER RESET")
    for epoch in range(train_epochs):
        print(f">>> EPOCH {epoch+1} <<<\n")
        train_loop()
        test_loop()
    print("TRAIN END")
    save_model()
    print("SAVED MODEL")

def run_inference(img:Image) -> int:
    if(model == None):
        raise Exception("No model instanciated")
    img = preprocess(img)
    img = to_tensor(img).to(device)
    
    with torch.no_grad():
        pred = model(img)
        #_, output = torch.max(pred, 1)
        inference = int(torch.max(pred.data, 1)[1].cpu().numpy())
        return inference
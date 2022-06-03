import threading
from nullbot import modules
from time import sleep

registered_threads = []

class LoopThread:
    """A thread for the watchdog that loops permanently"""
    def __init__(self, target, args:tuple=tuple(), daemon:bool=False, name:str=None) -> None:
        self.target = target
        self.args = args
        self.name = name
        self.thread = threading.Thread(target=self.task, args=args, name=self.name, daemon=daemon)
        self.running = False
        registered_threads.append(self)

    def task(self) -> None:
        self.running = True
        while( self.running ):
            try:
                self.target(*self.args)
            except Exception as e:
                self.running = False
                report_thread_crash(self, e)

    def start(self) -> None:
        self.thread.start()
    
    def stop(self) -> None:
        self.running = False


def shutdown_all_threads() -> None:
    for thread in registered_threads:
        thread.stop()

def report_thread_crash(thread:LoopThread, e:Exception) -> None:
    print(f"Thread {thread.name} crashed unexpectedly")
    print(f"Reason: {e}")
    # modules.get_controller().display_thread_failure()

# def test(i):
#    print(i)
#    sleep(2)
#    raise Exception("thi is an exception")

# t  = LoopThread(target=test, args=(1,), name="test", daemon=True)
# t.start()
# print(registered_threads)

# while( True ):
#    sleep(5)
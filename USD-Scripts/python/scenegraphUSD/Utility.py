import os,sys

class queue(object):
    def __init__(self,input):
        self.data = input
        if ( self.data ):
            self.front = self.data[0]
            self.rear = self.data[-1]
        else:
            self.front = self.rear = None            
    def knocked(self):
        '''
        Test if this queue is empty!
        '''
        if ( self.data ):
            return True
        else :
            return False
    def enQueue(self,element):
        '''
        Push the element into the end of this queue
        '''
        self.data.append(element)
        self.front = self.data[0]
        self.rear = self.data[-1]
    def deQueue(self):
        '''
        Pop first element of this queue
        '''
        if self.knocked():
            self.data = self.data[1:]
            if self.knocked(): # you need to check it twice beacuse you have changed self.data just a minute
                self.front = self.data[0]
                self.rear = self.data[-1]
            else :
                self.front = self.rear = None
        else :
            self.data = []
            self.front = self.rear = None 
    def enExpand(self,elements):
        '''
        Push a list into the end of this queue
        '''
        self.data += elements
        self.front = self.data[0]
        self.rear = self.data[-1]

def USDPluginLoading():
    import maya.cmds as cmds
    try:
        cmds.loadPlugin("pxrUsd.so")
    except ImportError:
        raise
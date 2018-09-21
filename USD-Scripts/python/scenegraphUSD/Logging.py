from datetime import datetime

def currentTime():
    return str(datetime.now().strftime('%H:%M:%S %Y-%m-%d'))

def scenegraphLogging(text):
        time = currentTime()
        print "[%s SceneGraphUSD] %s"%(time,text)

def scenegraphSeparater(text):
        time = currentTime()
        gap ="\n\n\n\n\n" 
        line = "##########################################################\n"
        content ="[%s SceneGraphUSD] %s"%(time,text)
        print gap+line+content

def scenegraphStepper(current,deno,text):
        time = currentTime()
        gap ="\n\n\n\n\n"
        # step = "[ *** %s\% *** ] "%( (current/deno)*100 )
        step = "[ *** %s/%s *** ] "%( current, deno)

        line = "##########################################################\n"
        content ="[%s SceneGraphUSD] %s"%(time,text)
        print gap+ step + line+content


if __name__ == "__main__":
    scenegraphStepper(1,10,"test")
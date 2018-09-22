/*
*/

#include <iostream>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <stdio.h>
#include <string>
#include <vector>


 #include <ImfChannelList.h>
#include <half.h>
#include <ImfArray.h>
#include <ImfOutputFile.h>

#include "exr_io.h"

using namespace std;

static void usage(const char * const program) {
    fprintf(stderr, "usage: %s <input.exr> <output.exr>\n", program);
    fprintf(stderr, "\t Auto crop EXR file into a new! \n");
    exit(1);
}

bool endswithdota(char *filename)
{
    bool result = false;
    std::string source = filename;
    if ( source.length() < 2)
    {
        if ( source == "A" )
            result = true;
        else result = false;
    }
    else
    {
        if(source.substr( source.length() - 2 ) == ".A")
        {
            // match
            result = true;
        }        
    }
    return result;
}


int main(int argc, char *argv[]) 
{
    const char *outfile = NULL;
    const char *infile = NULL;

    if (argc == 1) usage(argv[0]);
    for (int i = 1; i < argc; ++i) {
        if (!infile)
            infile = argv[i];
        else if (!outfile)
            outfile = argv[i];
        else
            usage(argv[0]);
    }
    printf("[Start Process] read file from : %s \n", infile);

    Array2D<half> rPixels;
    Array2D<half> gPixels;
    Array2D<half> bPixels;
    Array2D<float> aPixels;

    Box2i dataWindow;
    Box2i displayWindow;

    bool noAlpha = false;
    bool isCorpped = false;

    int minx,miny,maxx,maxy;
    int width,height;

    vector<string> floatPixelNameSet;
    Array < Array2D<float> > floatPixelSet;
    vector<string> halfPixelNameSet;
    Array < Array2D<half> > halfPixelSet;
    vector<string> uintPixelNameSet;
    Array < Array2D<uint> > uintPixelSet;

    // all the alpha data would be store here.
    vector<string> alphaNameSet;
    Array < Array2D<float> > alphaSet;
    
    // read pixles from exr image.
    try{
    InputFile inputFile (infile);
    Box2i dw = inputFile.header().dataWindow();
    Box2i dsw = inputFile.header().displayWindow();
    width  = dsw.max.x - dsw.min.x + 1;
    height = dsw.max.y - dsw.min.y + 1;
    dataWindow = dw;
    displayWindow = dsw;
    if (dw == dsw)
        isCorpped = false;
    else
        isCorpped = true;

    minx=dsw.max.x;
    miny=dsw.max.y;
    maxx=0;
    maxy=0;

    printf("[Data Window] min x:%i, min y:%i, max x:%i, max y: %i \n", dw.min.x, dw.min.y, dw.max.x, dw.max.y);
    printf("[Display Window] min x:%i, min y:%i, max x:%i, max y: %i \n", dsw.min.x, dsw.min.y, dsw.max.x, dsw.max.y);


    FrameBuffer readFrameBuffer;

    const ChannelList &channels = inputFile.header().channels();

    for (ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i)
    {
        // printf("Channels %s\n", i.name());
        switch (i.channel().type){
            case FLOAT:
                floatPixelNameSet.push_back(string(i.name()));
                break;
            case HALF:
                halfPixelNameSet.push_back(string(i.name()));
                break;
            case UINT:
                uintPixelNameSet.push_back(string(i.name()));
                break;
            default:
                floatPixelNameSet.push_back(string(i.name()));
                break;
        }
        if ( endswithdota((char*) i.name()) )
            alphaNameSet.push_back(string(i.name()));
    }

    if(alphaNameSet.empty())
        noAlpha = true;


    if (isCorpped || noAlpha)
    {
        printf("[Shut Down] no alpha:%s | has croped:%s \n", 
            noAlpha?"yes":"no",isCorpped?"yes":"no");
        return 0;
    }

    // read FLOAT RGB channel
    floatPixelSet.resizeErase (floatPixelNameSet.size());
    // printf("%s\n", "floatPixelSet");
    for (unsigned int it = 0; it < floatPixelNameSet.size(); ++it)
    {
        floatPixelSet[it].resizeErase (height, width);
        readFrameBuffer.insert (floatPixelNameSet[it],
                            Slice (FLOAT,
                                   (char *) (&floatPixelSet[it][0][0] -
                                             dw.min.x -
                                             dw.min.y * width),
                                   sizeof (floatPixelSet[it][0][0]) * 1,
                                   sizeof (floatPixelSet[it][0][0]) * width,
                                   1, 1,
                                   0.0));        
    }

    // read HALF RGB channel
    halfPixelSet.resizeErase (halfPixelNameSet.size());
    // printf("%s\n", "halfPixelSet");
    for (unsigned int it = 0; it < halfPixelNameSet.size(); ++it)
    {
        halfPixelSet[it].resizeErase (height, width);
        readFrameBuffer.insert (halfPixelNameSet[it],
                            Slice (HALF,
                                   (char *) (&halfPixelSet[it][0][0] -
                                             dw.min.x -
                                             dw.min.y * width),
                                   sizeof (halfPixelSet[it][0][0]) * 1,
                                   sizeof (halfPixelSet[it][0][0]) * width,
                                   1, 1,
                                   0.0));        
    }


    // read UINT RGB channel
    uintPixelSet.resizeErase (uintPixelNameSet.size());
    // printf("%s\n", "uintPixelSet");
    for (unsigned int it = 0; it < uintPixelNameSet.size(); ++it)
    {
        uintPixelSet[it].resizeErase (height, width);
        readFrameBuffer.insert (uintPixelNameSet[it],
                            Slice (UINT,
                                   (char *) (&uintPixelSet[it][0][0] -
                                             dw.min.x -
                                             dw.min.y * width),
                                   sizeof (uintPixelSet[it][0][0]) * 1,
                                   sizeof (uintPixelSet[it][0][0]) * width,
                                   1, 1,
                                   0.0));        
    }


    inputFile.setFrameBuffer (readFrameBuffer);
    inputFile.readPixels (dw.min.y, dw.max.y);
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "[Error] unable to read image file \"%s\": %s \n", infile, e.what());
        return 0;
    }

    if(!floatPixelNameSet.empty())
    {
      for (int x = 0; x < width; ++x)
      {
          for (int y = 0; y < height; ++y)
          {
              // printf("---------------------------------------\nx=%i,y=%i \n",x,y);
              if (floatPixelSet[0][y][x] > 0.0f)
              {
                  // printf("alpha %f \n",aPixels[y][x]);
                  minx = std::min(minx, x);
                  maxx = std::max(maxx, x);
                  miny = std::min(miny, y);
                  maxy = std::max(maxy, y);
              }
          }
      }
    }
    else if(!halfPixelNameSet.empty())
    {
      for (int x = 0; x < width; ++x)
      {
          for (int y = 0; y < height; ++y)
          {
              // printf("---------------------------------------\nx=%i,y=%i \n",x,y);
              if (halfPixelSet[0][y][x] > 0.0f)
              {
                  // printf("alpha %f \n",aPixels[y][x]);
                  minx = std::min(minx, x);
                  maxx = std::max(maxx, x);
                  miny = std::min(miny, y);
                  maxy = std::max(maxy, y);
              }
          }
      }
    }
    else
    {
      return 0;
    }

    try{
    printf("[Result Data Window] min x :%i,min y :%i,max x :%i,max y: %i \n", 
        minx, miny, maxx, maxy);
    Box2i resultDatawindow(V2i(minx,miny), V2i(maxx, maxy));

    // Write EXR
    Header header (width, height);

    header.dataWindow() = resultDatawindow;
    header.displayWindow() = displayWindow;

    FrameBuffer writeFrameBuffer;


    // WRITE FLOAT PIXEL DATA
    for (unsigned int it = 0; it < floatPixelNameSet.size(); ++it)
    {
        // printf("FLOAT channel %s\n", floatPixelNameSet[it].c_str());
        header.channels().insert (floatPixelNameSet[it], Channel (FLOAT));
        writeFrameBuffer.insert (floatPixelNameSet[it],
               Slice (FLOAT,
                  (char *) (&floatPixelSet[it][0][0] -
                                             dataWindow.min.x -
                                             dataWindow.min.y * width),
                  sizeof (floatPixelSet[it][0][0]) * 1,
                  sizeof (floatPixelSet[it][0][0]) * width,
                  1,
                  1));
    }

    // WRITE HALF PIXEL DATA
    for (unsigned int it = 0; it < halfPixelNameSet.size(); ++it)
    {
        // printf("HALF channel %s\n", halfPixelNameSet[it].c_str());
        header.channels().insert (halfPixelNameSet[it], Channel (HALF));
        writeFrameBuffer.insert (halfPixelNameSet[it],
               Slice (HALF,
                  (char *) (&halfPixelSet[it][0][0] -
                                             dataWindow.min.x -
                                             dataWindow.min.y * width),
                  sizeof (halfPixelSet[it][0][0]) * 1,
                  sizeof (halfPixelSet[it][0][0]) * width,
                  1,
                  1));
    }

    // WRITE UINT PIXEL DATA
    for (unsigned int it = 0; it < uintPixelNameSet.size(); ++it)
    {
        header.channels().insert (uintPixelNameSet[it], Channel (UINT));
        writeFrameBuffer.insert (uintPixelNameSet[it],
               Slice (UINT,
                  (char *) (&uintPixelSet[it][0][0] -
                                             dataWindow.min.x -
                                             dataWindow.min.y * width),
                  sizeof (uintPixelSet[it][0][0]) * 1,
                  sizeof (uintPixelSet[it][0][0]) * width,
                  1,
                  1));
    }


    OutputFile outputFile (outfile, header);

    outputFile.setFrameBuffer (writeFrameBuffer);
    printf("[Write EXR] save file to :%s \n", outfile);
    outputFile.writePixels (resultDatawindow.max.y - resultDatawindow.min.y + 1);
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "\n[Error] unable to write image file \"%s\": %s \n", infile, e.what());
        return 0;
    }
    return 0;
}
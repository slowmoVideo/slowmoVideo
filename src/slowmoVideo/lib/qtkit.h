/*
 * class to export a movie using QuickTime under OSX
*/

#include <QImage>
#import <QTKit/QTKit.h>

#include "video_enc.h"

class VideoQT : public VideoWriter{
    int mHeight;
    int mWidth;
    double movieFPS;
    NSString *codecSpec;
    NSString *qualitySpec;
    NSString *destPath;

    QTMovie* mMovie;
    NSDictionary *imageAttributes;
    QTTime duration;
    
public:
    VideoQT(int width,int height,double fps,const char *vcodec,const char* vquality,const char *filename);
    ~VideoQT();
    
    int writeFrame(const QImage& frame);
	int exportFrames(QString filepattern,int first,RenderTask_sV *progress);
};



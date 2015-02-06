/*
 * class to export a movie using QuickTime
 */
#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QtCore/QSettings>
#include <QImage>

#include "qtkit.h"
#include "video_enc.h"


#include "../project/renderTask_sV.h"

#pragma mark - 
#pragma mark cocoa bridge

// tools for qt 4.8
// convert pixamp <-> nsimage
static void drawImageReleaseData (void *info, const void *, size_t)
{
    delete static_cast<QImage *>(info);
}

CGImageRef qt_mac_image_to_cgimage(const QImage&img)
{
    QImage *image;
    if (img.depth() != 32)
        image = new QImage(img.convertToFormat(QImage::Format_ARGB32_Premultiplied));
    else
        image = new QImage(img);
    
    uint cgflags = kCGImageAlphaNone;
    switch (image->format()) {
        case QImage::Format_ARGB32_Premultiplied:
            cgflags = kCGImageAlphaPremultipliedFirst;
            break;
        case QImage::Format_ARGB32:
            cgflags = kCGImageAlphaFirst;
            break;
        case QImage::Format_RGB32:
            cgflags = kCGImageAlphaNoneSkipFirst;
        default:
            break;
    }
    cgflags |= kCGBitmapByteOrder32Host;
    CGDataProviderRef dataProvider = CGDataProviderCreateWithData(image,
                                                                  static_cast<const QImage *>(image)->bits(),
                                                                  image->byteCount(),
                                                                  drawImageReleaseData);
    CGColorSpaceRef colorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    
    
    CGImageRef cgImage = CGImageCreate(image->width(), image->height(), 8, 32,
                                       image->bytesPerLine(),
                                       colorspace,
                                       cgflags, dataProvider, 0, false, kCGRenderingIntentDefault);
    
    CFRelease(dataProvider);
    CFRelease(colorspace);
    return cgImage;
}

NSImage *toNSImage(const QImage& InImage)
{
    NSBitmapImageRep *bitmapRep = [[NSBitmapImageRep alloc] initWithCGImage: qt_mac_image_to_cgimage(InImage)];
    NSImage *image = [[NSImage alloc] init];
    [image addRepresentation:bitmapRep];
    [bitmapRep release];
    return image;
}

// end of tools


#pragma mark -

/* TODO :
 "-fps: Frames per second for final movie can be anywhere between 0.1 and 60.0.\n"
 "-height: If specified images are resized proportionally to height given.\n"
 "-codec: Codec to use to encode can be 'h264' 'photojpeg' 'raw' or 'mpv4'.\n"
 "-quality: Quality to encode with can be 'high' 'normal' 'low'.\n"
 "-quiet: Set to 'yes' to suppress output during encoding.\n"
 "-reverse: Set to 'yes' to reverse the order that images are displayed in the movie.\n"
 
 "DEFAULTS\n"
 "fps = 30\n"
 "height = original image size\n"
 "codec = h264\n"
 "quality = high\n\n"

 
 */

VideoQT::VideoQT(int width,int height,double fps,const char *vcodec,const char* vquality,const char *filename)
{
    NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
    movieFPS = fps;
    mMovie = nil;
    mHeight = height;
    mWidth = width;
    codecSpec = nil;
    qualitySpec = nil;

    
    NSDictionary *codec = [NSDictionary dictionaryWithObjectsAndKeys:
                           @"avc1", @"h264",
                           @"mpv4", @"mpv4",
                           @"jpeg", @"photojpeg",
                           @"raw ", @"raw", nil];

    NSDictionary *quality = [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSNumber numberWithLong:codecLowQuality], @"low",
                             [NSNumber numberWithLong:codecNormalQuality], @"normal",
                             [NSNumber numberWithLong:codecMaxQuality], @"high", nil];

    if (codecSpec == nil) {
        codecSpec = @"h264";
    }
    
    /*
     codecLosslessQuality          = 0x00000400,
     codecMaxQuality               = 0x000003FF,
     codecMinQuality               = 0x00000000,
     codecLowQuality               = 0x00000100,
     codecNormalQuality            = 0x00000200,
     codecHighQuality              = 0x00000300
     */

    if (qualitySpec == nil) {
        qualitySpec = @"high";
    }
    
    imageAttributes = [[NSDictionary dictionaryWithObjectsAndKeys:
                                     [codec objectForKey:codecSpec], QTAddImageCodecType,
                                     [quality objectForKey:qualitySpec], QTAddImageCodecQuality,
                                     [NSNumber numberWithLong:100000], QTTrackTimeScaleAttribute,
                                     nil] retain];
    long timeScale = 100000;
    long long timeValue = (long long) ceil((double) timeScale / fps);
    duration = QTMakeTime(timeValue, timeScale);
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    destPath = [[NSURL fileURLWithPath:[[NSString stringWithUTF8String:filename]
                                        stringByExpandingTildeInPath]] path];
    
    if (![destPath hasSuffix:@".mov"]) {
        fprintf(stderr, "Error: Output filename must be of type '.mov'\n");
        //return 1;
    }
    
    if ([fileManager fileExistsAtPath:destPath]) {
        fprintf(stderr, "Error: Output file already exists.\n");
        //return 1;
    }
    
    mMovie = [[QTMovie alloc] initToWritableFile:destPath error:NULL];
    if (mMovie == nil) {
        fprintf(stderr, "%s","Error: Unable to initialize QT object.\n");
        //return 1;
    }
    [mMovie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieEditableAttribute];
    [localpool drain];
}

//
// add one frame to the movie
int VideoQT::writeFrame(const QImage& frame)
{
    NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
#if 1
    NSImage* nsimage =toNSImage(frame);
#else
    NSBitmapImageRep* imageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&imagedata
                                                                         pixelsWide:width
                                                                         pixelsHigh:height
                                                                      bitsPerSample:8
                                                                    samplesPerPixel:4
                                                                           hasAlpha:YES
                                                                           isPlanar:NO
                                                                     colorSpaceName:NSDeviceRGBColorSpace
                                                                       bitmapFormat:NSAlphaFirstBitmapFormat
                                                                        bytesPerRow:argbimage->widthStep
                                                                       bitsPerPixel:32]  ;
    NSImage* nsimage = [[NSImage alloc] init];
    [nsimage addRepresentation:imageRep];
#endif
    // maybe should resize here ?
    
    [mMovie addImage:nsimage
        forDuration:duration
     withAttributes:imageAttributes];
    
    if (![mMovie updateMovieFile]) {
        fprintf(stderr, "Didn't successfully update movie file. \n" );
	return 1;
    }

    
    //[imageRep release];
    [nsimage release];
    [localpool drain];
    return 0;
}

#pragma mark - 

int VideoQT::exportFrames(QString filepattern,int first,RenderTask_sV *progress)
{
	NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
	NSString *inputPath;
    NSArray *imageFiles;
    NSError *err;
    NSImage *image;
    
    NSString *fullFilename;
        
	qDebug() << "exporting frame from : " << filepattern << " to " << destPath;
	NSLog(@"export to @%", destPath);
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString* inputdir = [[NSURL fileURLWithPath:[[NSString stringWithUTF8String:filepattern.toStdString().c_str()]
                    stringByExpandingTildeInPath]] path];
    inputPath = [inputdir  stringByDeletingLastPathComponent];
                
    imageFiles = [fileManager contentsOfDirectoryAtPath:inputPath error:&err];
    imageFiles = [imageFiles sortedArrayUsingSelector:@selector(localizedStandardCompare:)];
    
     for (NSString *file in imageFiles) {
        fullFilename = [inputPath stringByAppendingPathComponent:file];
        if ([[fullFilename pathExtension] caseInsensitiveCompare:@"jpeg"] == NSOrderedSame ||
            [[fullFilename pathExtension] caseInsensitiveCompare:@"png"] == NSOrderedSame ||
            [[fullFilename pathExtension] caseInsensitiveCompare:@"jpg"] == NSOrderedSame) {
            NSAutoreleasePool *innerPool = [[NSAutoreleasePool alloc] init];
            image = [[NSImage alloc] initWithContentsOfFile:fullFilename];
        
        	//NSLog(@"adding %@",fullFilename);
        	
        	[mMovie addImage:image
       			 forDuration:duration
     			  withAttributes:imageAttributes];
    
    		if (![mMovie updateMovieFile]) {
        			fprintf(stderr, "Didn't successfully update movie file. \n" );
					return 1;
			}
			
			// TODO:
    		progress->stepProgress();
            [image release];
            [innerPool release];
        }
    }
    
    [localpool drain];
    return 0;
}

//
// close the movie file
VideoQT::~VideoQT()
{
    NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];
    [mMovie updateMovieFile];
    
    [mMovie release];
    [qualitySpec release];
    [codecSpec release];
    // TODO: [destPath release];
    [localpool drain];
}

#pragma mark -  
#pragma mark C/C++ bridge

VideoWriter* CreateVideoWriter_QT ( const char* filename, int width, int height, double fps,const char* codec)
{
	VideoQT*  driver=  new VideoQT(width,height,fps,0,0,filename);
	return driver;
}

#if 0
CGImageRef qt_mac_image_to_cgimage(const QImage &image)
{
    int bitsPerColor = 8;
    int bitsPerPixel = 32;
    if (image.depth() == 1) {
        bitsPerColor = 1;
        bitsPerPixel = 1;
    }
    QCFType<CGDataProviderRef> provider =
    CGDataProviderCreateWithData(0, image.bits(), image.bytesPerLine() * image.height(),
                                 0);
    
    uint cgflags = kCGImageAlphaPremultipliedFirst;
#ifdef kCGBitmapByteOrder32Host //only needed because CGImage.h added symbols in the minor version
    cgflags |= kCGBitmapByteOrder32Host;
#endif
    
    CGImageRef cgImage = CGImageCreate(image.width(), image.height(), bitsPerColor, bitsPerPixel,
                                       image.bytesPerLine(),
                                       QCoreGraphicsPaintEngine::macGenericColorSpace(),
                                       cgflags, provider,
                                       0,
                                       0,
                                       kCGRenderingIntentDefault);
    
    return cgImage;
}

void * /*NSImage */qt_mac_create_nsimage(const QPixmap &pm)
{
    QMacCocoaAutoReleasePool pool;
    if(QCFType<CGImageRef> image = pm.toMacCGImageRef()) {
        NSImage *newImage = 0;
        NSRect imageRect = NSMakeRect(0.0, 0.0, CGImageGetWidth(image), CGImageGetHeight(image));
        newImage = [[NSImage alloc] initWithSize:imageRect.size];
        [newImage lockFocus];
        {
            CGContextRef imageContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
            CGContextDrawImage(imageContext, *(CGRect*)&imageRect, image);
        }
        [newImage unlockFocus];
        return newImage;
    }
    return 0;
}

QPixmap pixmap = icon.pixmap(size);
CGImageRef cgImage = pixmap.toMacCGImageRef();//compile errors here
image = [[NSImage alloc] initWithCGImage:cgImage size:NSZeroSize];
CFRelease(cgImage);

Creates a CGImageRef equivalent to the QPixmap. Returns the CGImageRef handle.

It is the caller's responsibility to release the CGImageRef data after use.

Warning: This function is only available on Mac OS X.

This function was introduced in Qt 4.2.

#endif

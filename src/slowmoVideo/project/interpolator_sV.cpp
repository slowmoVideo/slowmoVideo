#include "interpolator_sV.h"
#include "abstractFrameSource_sV.h"
#include "../lib/flowField_sV.h"
#include "../lib/interpolate_sV.h"
#include <QtCore/QObject>

#define MIN_FRAME_DIST .001

QImage Interpolator_sV::interpolate(Project_sV *pr, float frame, const RenderPreferences_sV &prefs)
throw(FlowBuildingError, InterpolationError)
{
    if (frame > pr->frameSource()->framesCount()) {
        throw InterpolationError(QObject::tr("Requested frame %1: Not within valid range. (%2 frames)")
                                 .arg(frame).arg(pr->frameSource()->framesCount()));
    }
    if (frame-floor(frame) > MIN_FRAME_DIST) {
        
        QImage left = pr->frameSource()->frameAt(floor(frame), prefs.size);
        QImage right = pr->frameSource()->frameAt(floor(frame)+1, prefs.size);
        QImage out(left.size(), QImage::Format_ARGB32);
        
        /// Position between two frames, on [0 1]
        const float pos = frame-floor(frame);
        
        switch (prefs.interpolation) {
            case InterpolationType_Twoway: {
                FlowField_sV *forwardFlow = pr->requestFlow(floor(frame), floor(frame)+1, prefs.size);
                FlowField_sV *backwardFlow = pr->requestFlow(floor(frame)+1, floor(frame), prefs.size);
                
                Q_ASSERT(forwardFlow != NULL);
                Q_ASSERT(backwardFlow != NULL);
                
                if (forwardFlow == NULL || backwardFlow == NULL) {
                    qDebug() << "No flow received!";
                    Q_ASSERT(false);
                }
                
                Interpolate_sV::twowayFlow(left, right, forwardFlow, backwardFlow, pos, out);
                delete forwardFlow;
                delete backwardFlow;
                
            }
                break;
            case InterpolationType_TwowayNew: {
                FlowField_sV *forwardFlow = pr->requestFlow(floor(frame), floor(frame)+1, prefs.size);
                FlowField_sV *backwardFlow = pr->requestFlow(floor(frame)+1, floor(frame), prefs.size);
                
                Q_ASSERT(forwardFlow != NULL);
                Q_ASSERT(backwardFlow != NULL);
                
                if (forwardFlow == NULL || backwardFlow == NULL) {
                    qDebug() << "No flow received!";
                    Q_ASSERT(false);
                }
                
                Interpolate_sV::newTwowayFlow(left, right, forwardFlow, backwardFlow, pos, out);
                delete forwardFlow;
                delete backwardFlow;
                
            }
                break;
            case InterpolationType_Forward: {
                FlowField_sV *forwardFlow = pr->requestFlow(floor(frame), floor(frame)+1, prefs.size);
                
                Q_ASSERT(forwardFlow != NULL);
                
                if (forwardFlow == NULL) {
                    qDebug() << "No flow received!";
                    Q_ASSERT(false);
                }
                
                Interpolate_sV::forwardFlow(left, forwardFlow, pos, out);
                delete forwardFlow;
                
            }
                break;
            case InterpolationType_ForwardNew: {
                FlowField_sV *forwardFlow = pr->requestFlow(floor(frame), floor(frame)+1, prefs.size);
                
                Q_ASSERT(forwardFlow != NULL);
                
                if (forwardFlow == NULL) {
                    qDebug() << "No flow received!";
                    Q_ASSERT(false);
                }
                
                Interpolate_sV::newForwardFlow(left, forwardFlow, pos, out);
                delete forwardFlow;
                
            }
                break;
            case InterpolationType_Bezier: {
                FlowField_sV *currNext = pr->requestFlow(floor(frame)+2, floor(frame)+1, prefs.size); // Allowed to be NULL
                FlowField_sV *currPrev = pr->requestFlow(floor(frame)+0, floor(frame)+1, prefs.size);
                
                Q_ASSERT(currPrev != NULL);
                
                Interpolate_sV::bezierFlow(left, right, currPrev, currNext, pos, out);
                
                delete currNext;
                delete currPrev;
                
            }
                break;
            case InterpolationType_None: {
                //qDebug() << "Simple interpolation type!";
                Interpolate_sV::simpleinterpolate(left, right, pos, out);
            }
                break;
            case InterpolationType_Nearest: {
                Interpolate_sV::nearestinterpolate(left, right, pos, out);
            }
                break;
            default : {
                qDebug() << "Unsupported interpolation type!";
                Q_ASSERT(false);
            }
        }
        return out;
    } else {
        qDebug() << "No interpolation necessary.";
        return pr->frameSource()->frameAt(floor(frame), prefs.size);
    }
}

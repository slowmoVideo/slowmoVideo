
class QImage;

class InterpolateSV {

 public:

    struct Movement {
	float moveX;
	float moveY;
	float mult;
    };

    static void twowayFlow(const QImage& left, const QImage& right, const QImage& flowForward, const QImage& flowBackward, float pos, QImage output);
    static void forwardFlow(const QImage& left, const QImage& right, const QImage& flow, float pos, QImage output);
};

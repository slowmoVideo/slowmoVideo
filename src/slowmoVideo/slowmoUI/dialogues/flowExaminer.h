#ifndef FLOWEXAMINER_H
#define FLOWEXAMINER_H

#include <QDialog>

namespace Ui {
    class FlowExaminer;
}
class Project_sV;
class FlowField_sV;

class FlowExaminer : public QDialog
{
    Q_OBJECT

public:
    explicit FlowExaminer(Project_sV *project, QWidget *parent = 0);
    ~FlowExaminer();

    void examine(int leftFrame);

private:
    Ui::FlowExaminer *ui;

    Project_sV *m_project;
    FlowField_sV *m_flowLR;
    FlowField_sV *m_flowRL;

	// current frame
	int frame;
	// color flow amplification
	float m_boost;
private slots:
    void slotMouseMoved(float x, float y);
    
public slots:
	void newAmplification(int val);
	
protected:
	void wheelEvent(QWheelEvent *);
    void keyPressEvent(QKeyEvent *event);
};

#endif // FLOWEXAMINER_H

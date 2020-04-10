
#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

class progressDialog {

	public:
    progressDialog() {};
    virtual ~progressDialog() {}
    
    virtual void updateProgress() = 0;
};

#endif


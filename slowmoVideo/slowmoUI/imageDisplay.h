/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include <QFrame>

/**
  \brief Simple image display.

  Images can be scaled to the frame size and exported to a file
  via the context menu.
  */
class ImageDisplay : public QFrame
{
    Q_OBJECT
public:
    explicit ImageDisplay(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~ImageDisplay();

    /// \return The image that is currently displayed
    const QImage& image() const;

public slots:
    /// Loads the given image; does \em not call repaint()!
    void loadImage(const QImage img);

protected slots:
    virtual void paintEvent(QPaintEvent *e);
    virtual void contextMenuEvent(QContextMenuEvent *e);

private:
    QImage m_image;

    QAction *m_aScaling;
    QAction *m_aExportImage;

private slots:
    void slotExportImage();

};

#endif // IMAGEDISPLAY_H

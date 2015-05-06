// a simple picture view base on qt
// 2015, by liangyaozhan

#include <QApplication>
#include <QImage>
#include <QLayout>
#include <QPaintEvent>
#include <QGraphicsView>
#include <QMessageBox>
#include <QFileDialog>
#include "QAbstractButton"

#include <iostream>
#include <stdio.h>


// clip logic
class area_clip
{
public:
    int src_origin_w;
    int src_origin_h;

    int src_scaled_w;
    int src_scaled_h;

    int clip_left;
    int clip_top;
    int clip_right;
    int clip_bottom;

    int draw_x;
    int draw_y;
    int screen_width;
    int screen_heigh;
    int focus_x;
    int focus_y;
    float percent;

    int focus_scaled_x;
    int focus_scaled_y;


public:
#define __CO(t) t(0)
    area_clip():
        __CO(src_origin_w),
        __CO(src_origin_h),
        __CO(src_scaled_w),
        __CO(src_scaled_h),
        __CO(clip_left),
        __CO(clip_top),
        __CO(clip_right),
        __CO(clip_bottom),
        __CO(draw_x),
        __CO(draw_y),
        __CO(screen_width),
        __CO(screen_heigh),
        __CO(focus_x),
        __CO(focus_y),
        __CO(focus_scaled_x),
        __CO(focus_scaled_y)
#undef __CO
        {
            percent = 1.0;
            m_x = 0;
            m_y = 0;
            m_x0 = 0;
            m_y0 = 0;
        }

    void set_src_size( int sw, int sh )
        {
            this->src_origin_w = sw;
            this->src_origin_h = sh;
        }

    void set_dst_size( int dw, int dh )
        {
            this->screen_width = dw;
            this->screen_heigh = dh;
        }

    void set_fix_size( void )
        {
            float rs = (float)this->src_origin_h / (float)this->src_origin_w;
            float rd = (float)this->screen_heigh / (float)this->screen_width;

            move_enter(0, 0);
            percent = 1;
            draw_x = 0;
            draw_y = 0;
            clip_left = 0;
            clip_top = 0;
            m_x = 0;
            m_y = 0;
            m_x0 = 0;
            m_y0 = 0;

            if ( rd > rs )
            {
                f( (float)this->screen_width/this->src_origin_w, 0, 0 );
            } else {
                f( (float)this->screen_heigh/this->src_origin_h, 0, 0 );
            }
        }

    void update_clip_rect( void )
        {
            clip_left -= m_x;
            clip_right -= m_x;

            clip_top -= m_y;
            clip_bottom -= m_y;
            m_x = 0;
            m_y = 0;
        }

    int m_x0;
    int m_y0;
    int m_x;
    int m_y;

    void move_enter( int x, int y )
        {
            m_x0 = x;
            m_y0 = y;
            m_x = 0;
            m_y = 0;
        }
    void move_move( int x, int y )
        {
            int _x = x - m_x0;
            int _y = y - m_y0;
            if ( clip_left - _x >= 0
                 && clip_right - _x < src_scaled_w
                )
            {
                m_x = _x;
            }
            if (
                  clip_top - _y >= 0
                 && clip_bottom - _y < src_scaled_h
                )
            {
                m_y = _y;
            }
        }
    void move_leave( int x, int y)
        {
            update_clip_rect();
        }

    void f( float percent, int focus_x, int focus_y )
        {
            if ( !(focus_x < screen_width && focus_y < screen_heigh
                   && focus_x >= draw_x && focus_y >= draw_y) )
            {
                focus_x = draw_x + (clip_right - clip_left)/2;
                focus_y = draw_y + (clip_bottom - clip_top)/2;
            }

            int x = focus_x;
            int y = focus_y;
            float last_percent = this->percent;
            if ( last_percent )
            {
                focus_x = (focus_x - draw_x + clip_left) / last_percent;
                focus_y = (focus_y - draw_y + clip_top)  / last_percent;
            }

            this->focus_x = focus_x; // original
            this->focus_y = focus_y; // original
            this->percent = percent;

            // resize
            src_scaled_w = src_origin_w * percent;
            src_scaled_h = src_origin_h * percent;

            focus_scaled_x = focus_x * percent; // on source
            focus_scaled_y = focus_y * percent; // on source

            int clip_x, clip_y;

            clip_x      = focus_scaled_x - x;
            clip_y      = focus_scaled_y - y;
            clip_left   = clip_x;
            clip_top    = clip_y;
            clip_right  = clip_x + screen_width;
            clip_bottom = clip_y + screen_heigh;

            draw_x = 0;
            draw_y = 0;

            if ( clip_left < 0 )
            {
                draw_x = -clip_left;
                clip_left = 0;
            }
            if ( clip_top < 0 )
            {
                draw_y = -clip_top;
                clip_top = 0;
            }
        }
};

class image_viewer: public QWidget
{
public:
    image_viewer( const char *filename )
        {
            load_image( filename );
        }
    ~image_viewer(){}

    void load_image( const char *filename )
        {
            int sw, sh;

            int ret = this->img.load( QString::fromLocal8Bit(filename) );

            if ( !ret )
            {
                QMessageBox *ms =  new QMessageBox(QMessageBox::Information, QString("qvimage failed"), QString::fromLocal8Bit( filename ) );
                ms->setStandardButtons(QMessageBox::Ok);
                ms->show();
            }

            sw = this->img.width();
            sh = this->img.height();
            this->ac.set_src_size( sw, sh );
            this->ac.set_dst_size( this->width(), this->height() );
            this->ac.set_fix_size();
        }

private:
    area_clip ac;
    QImage img;
    QImage scaled_img;// = this->img.scaled( ac.src_scaled_w, ac.src_scaled_h );
    
    void update( void )
    {
        this->scaled_img = this->img.scaled( ac.src_scaled_w, ac.src_scaled_h );
        QWidget::update();
    }

    void paintEvent( QPaintEvent *event );
    void wheelEvent(QWheelEvent *);
    void resizeEvent( QResizeEvent *r )
        {
            this->ac.set_dst_size( r->size().width(), r->size().height() );
            this->ac.set_fix_size();
            ac.f( ac.percent, ac.focus_x, ac.focus_y );
            update();
        }

    void mouseDoubleClickEvent(QMouseEvent *event)
        {
            ac.set_fix_size();
            update();
        }

    void mousePressEvent(QMouseEvent *e)
        {
            ac.move_enter(e->x(), e->y());
            QWidget::update();
        }

    void mouseReleaseEvent(QMouseEvent *e)
        {
            ac.move_leave(e->x(), e->y());
            QWidget::update();
        }


    void mouseMoveEvent(QMouseEvent *e)
        {
            ac.move_move(e->x(), e->y());
            QWidget::update();
        }
};

void image_viewer::paintEvent( QPaintEvent *event )
{
    QPainter pt(this);

    pt.drawImage( ac.draw_x, ac.draw_y, scaled_img, ac.clip_left - ac.m_x, ac.clip_top-ac.m_y, ac.clip_right-ac.m_x, ac.clip_bottom-ac.m_y );
}

void image_viewer::wheelEvent( QWheelEvent *wheel )
{
    float f;
    if ( wheel->delta() > 0 )
    {
        f = ac.percent * 0.1;
    } else {
        f = -ac.percent * 0.1;
    }
    ac.f( ac.percent+f, wheel->x(), wheel->y() );
    update();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if ( argc < 2 )
    {
        fprintf(stderr, "error, no file\n");
        QMessageBox *ms =  new QMessageBox(QMessageBox::Information, QString("qvimage"), QString("image file not specified.") );
        ms->setStandardButtons(QMessageBox::Ok|QMessageBox::Abort);
        ms->show();
        app.exec();
        return -1;
    }
    image_viewer iv(argv[1]);
    iv.show();

    app.exec();
    return 0;
}


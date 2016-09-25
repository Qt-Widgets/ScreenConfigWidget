#ifndef SCREENCONFIGWIDGET_H
#define SCREENCONFIGWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRect>
#include <QResizeEvent>
#include <QVBoxLayout>

#include <assert.h>
#include <stdexcept>
#include <list>

#include <Magick++.h>

/**
 * @brief Store the selection state and the geometry (scale 1) for each border
 */
struct Border {
    Magick::Geometry geometry;///< current border geometry (in pixels, scale 1)
    bool isSelected = false;///< whether this border is currently selected

    /**
     * @brief Create a possibly scaled QRect representation of this border for easy drawing
     */
    QRect qRect(double scale = 1.0) const {
        return QRect(QPoint(geometry.xOff() * scale, geometry.yOff() * scale),
                     QSize(geometry.width() * scale, geometry.height()) * scale);
    }
};

struct Monitor {
    QRect boundingRectangle(double scale = 1.0) const {
        size_t leftPos = top.geometry.xOff() * scale;
        size_t topPos = top.geometry.yOff() * scale;


        size_t width = top.geometry.width() * scale;
        size_t height = (left.geometry.height() + 2 * top.geometry.height()) * scale;

        return QRect(leftPos, topPos, width, height);
    }

    Monitor(const QString name, size_t width, size_t height,
        size_t xOffset, size_t yOffset,
        size_t letterboxOffsetX, size_t letterboxOffsetY) :
        mName(name),
        mWidth(width), mHeight(height),
        mXOffset(xOffset), mYOffset(yOffset),
        mVerticalLetterboxBarWidth(letterboxOffsetX), mHorizontalLetterboxBarHeight(letterboxOffsetY)
    {
        updateGeometry();
    }

    void updateGeometry()
    {
        left.geometry = Magick::Geometry(
            BORDER_WIDTH, //width
            mHeight - 2 * BORDER_WIDTH - (2 * mHorizontalLetterboxBarHeight), //height
            mVerticalLetterboxBarWidth + mXOffset + 0, //x offset
            mHorizontalLetterboxBarHeight + mYOffset + BORDER_WIDTH);// y offset

        right.geometry = Magick::Geometry(
            BORDER_WIDTH, //width
            mHeight - 2 * BORDER_WIDTH - (2 * mHorizontalLetterboxBarHeight), //height
            (-mVerticalLetterboxBarWidth) + mXOffset + mWidth - BORDER_WIDTH, //x offset
            mHorizontalLetterboxBarHeight + mYOffset + BORDER_WIDTH);// y offset


        top.geometry = Magick::Geometry(
            mWidth - (2 * mVerticalLetterboxBarWidth), //width
            BORDER_WIDTH, //height
            mVerticalLetterboxBarWidth + mXOffset + 0, //x offset
            mHorizontalLetterboxBarHeight + mYOffset + 0);// y offset

        bottom.geometry = Magick::Geometry(
            mWidth - (2 * mVerticalLetterboxBarWidth), //width
            BORDER_WIDTH, //height
            mVerticalLetterboxBarWidth + mXOffset + 0, //x offset
            (-mHorizontalLetterboxBarHeight) + mYOffset + mHeight - BORDER_WIDTH);// y offset
    }

    const Border& operator[] (size_t i) const {
        switch(i){
            case 0: return bottom;
            case 1: return right;
            case 2: return top;
            case 3: return left;
            default:
            assert(i < 4);
            if(i >= 4) throw std::invalid_argument("invalid index");
            return bottom;
        }
    }

    Border& operator[] (size_t i) {
        switch(i){
            case 0: return bottom;
            case 1: return right;
            case 2: return top;
            case 3: return left;
            default:
            assert(i < 4);
            if(i >= 4) throw std::invalid_argument("invalid index");
            return bottom;
        }
    }

    const QString& getName() const {
        return mName;
    }

    void setPosition(const QPoint& targetPosition){
        // calculate the delta (-> target - current) to the position of the monitor, so we can reuse move()
        move(targetPosition - boundingRectangle().topLeft());
    }

    void move(const QPoint& delta){
        // move each border
        for(int i = 0; i < 4; i++){
            this->operator [](i).geometry.xOff(this->operator [](i).geometry.xOff() + delta.x());
            this->operator [](i).geometry.yOff(this->operator [](i).geometry.yOff() + delta.y());
        }
    }

    bool isSelected = false;

private:
    Border bottom, right, top, left;///< border geometry and selection state

    QString mName;///< the identification of this monitor

    size_t mWidth;///< screen geometry in pixels
    size_t mHeight;///< screen geometry in pixels
    size_t mXOffset;///< screen offset in pixels
    size_t mYOffset;///< screen offset in pixels
    size_t mVerticalLetterboxBarWidth;///< the height of the horizontal letterbox bars
    size_t mHorizontalLetterboxBarHeight;///< the width of the vertical letterbox bars

    const size_t BORDER_WIDTH = 16;///< how wide each border should be
};

class Screen {
    std::list<Monitor> mMonitorList; ///< list of all the known monitors
    double mScale = 1.0 / 10.0;

    bool monitorExists(const QString& name){
        bool exists = false;

        for(auto& m : mMonitorList)
            if(m.getName() == name)
                exists = true;

        return exists;
    }

public:
    const QString currentlySelectedMonitor(){
        for(auto m : mMonitorList)
            if(m.isSelected)
                return m.getName();
        return "HASHTAG_GOOD_CODING_NO_MONITOR_SELECTED";
    }

    void drawText(QPainter& painter, const Monitor& m){
        QRect bounding = m.boundingRectangle();
        painter.drawText(
                    m.boundingRectangle(mScale),
                    Qt::AlignCenter,
                    QString("%1\n%2x%3\n%4+%5")
                        .arg(m.getName())
                        .arg(bounding.width())
                        .arg(bounding.height())
                        .arg(bounding.left())
                        .arg(bounding.top()));
    }

    void drawBorders(QPainter& painter){
        // draw all monitors
        for(const Monitor& monitor : mMonitorList){
            // draw all borders
            for(size_t i = 0; i < 4; i++){
                QColor color = monitor[i].isSelected ? QColor::fromRgb(0, 255, 0) : QColor::fromRgb(255, 255, 255);
                painter.setPen(color);

                // draw a scaled down version of the borders
                painter.drawRect(monitor[i].qRect(mScale));

                // draw info text
                drawText(painter, monitor);
            }
        }
    }

    void drawBoundingRectangle(QPainter& painter){
        // draw all monitors
        for(const Monitor& monitor : mMonitorList){
            QColor fillColor;
            if(monitor.isSelected)
                fillColor = QColor::fromRgb(255, 180, 180);
            else
                fillColor = QColor::fromRgb(180, 180, 180);

            // draw a scaled down version of the bounding rectangle
            painter.fillRect(monitor.boundingRectangle(mScale), fillColor);

            // draw info text
            drawText(painter, monitor);
        }
    }

    void deleteMonitor(const QString& name){
        mMonitorList.remove_if([name](Monitor& m){ return m.getName() == name; });
    }



    bool addMonitor(const QString& name, int xRes, int yRes, int xOff = 0, int yOff = 0){
        // allow unique names only
        if(monitorExists(name))
            return false;

        // add monitor
        mMonitorList.push_back(Monitor(name, xRes, yRes, xOff, yOff, 0, 0));

        // return true
        return true;
    }

    void moveMonitors(const QString& selection, const QPoint& target, const QPoint& source, const QRect& bounding){
        selectSingleMonitor(selection);

        for(Monitor& monitor : mMonitorList)
            if(monitor.isSelected)
                snap(monitor, target, source, bounding);
    }

    /**
     * @brief Snap monitor to points of interest instead of straight moving
     */
    void snap(Monitor& snapping, const QPoint& target, const QPoint& /*source*/, const QRect& masterBounding){
        QRect snappingRect = snapping.boundingRectangle();
        QRect snappingRectMoved(snappingRect);
        snappingRectMoved.moveTo(target / mScale);

        // this would probably be the way to move by delta, if i could figure out how tf to get it working
        //QRect monMoved = mon.translated((target - source));

        // poi: a) within rectangle  b) to main border  c) to other monitors

        // POI b) snap to main border
        double heightTreshold = .05 * masterBounding.height();
        double widthTreshold = .05 * masterBounding.width();

        /*
         *
         * MAIN BORDER SNAPPING
         *
         */

        // stay greater equal zero
        if(snappingRectMoved.left() < widthTreshold)
            snappingRectMoved.moveLeft(0);

        if(snappingRectMoved.top() < heightTreshold)
            snappingRectMoved.moveTop(0);

        // stay smaller equal maximum
        if((masterBounding.right() / mScale - snappingRectMoved.right()) < widthTreshold)
            snappingRectMoved.moveRight(masterBounding.right() / mScale);

        if((masterBounding.bottom() / mScale - snappingRectMoved.bottom()) < heightTreshold)
            snappingRectMoved.moveBottom(masterBounding.bottom() / mScale);

        /*
         *
         * OTHER MONITOR SNAPPING
         *
         */

        for(const Monitor& other : mMonitorList){
            // we are only interested in the other monitors
            if(other.getName() == snapping.getName()) continue;
            QRect otherRect = other.boundingRectangle();
            // enlarge the other monitors rectangle to check for near collisions
            QRect otherTestRect = otherRect.adjusted(-widthTreshold / 2, -heightTreshold / 2, widthTreshold / 2, heightTreshold / 2);

            // check for collision...
            if(otherTestRect.intersects(snappingRectMoved)){
                QRect intersection = otherTestRect.intersected(snappingRectMoved);
                // the intersection occured when moving right to left or vice versa
                if(intersection.height() > intersection.width()) {
                    // snapping monitor is to the right
                    if(snappingRectMoved.right() > otherRect.right())
                        snappingRectMoved.moveLeft(otherRect.right());
                    // snapping monitor is to the left
                    else if (snappingRectMoved.right() < otherRect.right())
                        snappingRectMoved.moveRight(otherRect.left() - 1);
                }
                // the intersection occured when moving top to down or vice versa
                else {
                    // snapping monitor is to the top
                    if(snappingRectMoved.top() < otherRect.top())
                        snappingRectMoved.moveBottom(otherRect.top() - 1);
                    // snapping monitor is to the bottom
                    else if (snappingRectMoved.top() > otherRect.top())
                        snappingRectMoved.moveTop(otherRect.bottom());
                }
            }
        }

        snapping.setPosition(snappingRectMoved.topLeft());
    }

    void selectSingleMonitor(const QString& selection){
        // select only the monitor named like the selection
        for(Monitor& m : mMonitorList)
            m.isSelected = m.getName() == selection;
    }

    const QString getMonitor(const QPoint& pos) const {
        // select only the correctly named monitor
        for(const Monitor& m : mMonitorList)
            if(m.boundingRectangle(mScale).contains(pos))
                return m.getName();
        return "";
    }
};

class ScreenDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScreenDisplayWidget(QWidget *parent = 0) : QWidget(parent) {
        mScreen = new Screen();
    }

    const QString currentlySelectedMonitor(){
        return mScreen->currentlySelectedMonitor();
    }

    void deleteMonitor(const QString& name){
        mScreen->deleteMonitor(name);
        repaint();
    }

    bool addMonitor(const QString& name, int xRes, int yRes, int xOff = 0, int yOff = 0){
        bool added = mScreen->addMonitor(name, xRes, yRes, xOff, yOff);
        repaint();
        return added;
    }

    enum struct DisplayMode {
        BackgroundRectangle,
        BordersOnly
    };

    void setDisplayMode(DisplayMode dm){
        mDisplayMode = dm;
        repaint();
    }

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE {
        QPainter painter(this);
        //
        painter.fillRect(this->rect(), QColor::fromRgb(255, 255, 255));
        if(mDisplayMode == DisplayMode::BackgroundRectangle)
            mScreen->drawBoundingRectangle(painter);
        else if(mDisplayMode == DisplayMode::BordersOnly)
            mScreen->drawBorders(painter);
        else
            assert(false && "u fukin wot m8");// unknown enum element
    }

    void mousePressEvent(QMouseEvent *e) {
        mLastMousePosition = e->pos();
        mClickedMonitor = mScreen->getMonitor(mLastMousePosition);
        mMouseMoved = false;
    }

    void mouseMoveEvent(QMouseEvent *e) {
        mMouseMoved = true;
        mScreen->moveMonitors(mClickedMonitor, e->pos(), mLastMousePosition, this->rect());

        update();
    }

    void mouseReleaseEvent(QMouseEvent *e) {
        // save the last mouseposition
        mLastMousePosition = e->pos();

        // a monitor is no longer clicked
        mClickedMonitor = "";

        // if the mouse moved, this was not a click event
        if(mMouseMoved) return;

        // get clicked monitor
        QString selected = mScreen->getMonitor(e->pos());

        // select clicked monitor
        mScreen->selectSingleMonitor(selected);

        // update screen
        update();
    }

private:
    bool mMouseMoved = false;
    QString mClickedMonitor;
    QPoint mLastMousePosition;
    DisplayMode mDisplayMode = DisplayMode::BackgroundRectangle;
    Screen* mScreen;
};

class ScreenConfigLayout : public QWidget {
    Q_OBJECT
public:
    explicit ScreenConfigLayout(QWidget *parent = 0) : QWidget(parent) {
        // set our main layout manager
        mMainLayout = new QVBoxLayout(this->parentWidget());
        setLayout(mMainLayout);

        // create layout
        layout_();

        // connect to buttons
        connectSignals();
    }

    void connectSignals(){
        // connect button click signals
        connect(mAddButton, SIGNAL(clicked()), this, SLOT(onAddButton()));
        connect(mDeleteButton, SIGNAL(clicked()), this, SLOT(onDeleteButton()));
    }

    void layout_(){
        // monitor add layout
        QHBoxLayout* monitorLayout = new QHBoxLayout(this->parentWidget());
        mMainLayout->addLayout(monitorLayout);

        // line edit widgets for entering resolution and name
        mNameInput = new QLineEdit("name", this->parentWidget());
        mHorizontalResolutionInput = new QLineEdit("1366", this->parentWidget());
        mVerticalResolutionInput = new QLineEdit("768", this->parentWidget());

        monitorLayout->addWidget(mNameInput);
        monitorLayout->addWidget(mHorizontalResolutionInput);
        monitorLayout->addWidget(mVerticalResolutionInput);

        // add button
        mAddButton = new QPushButton("Add monitor", this->parentWidget());
        monitorLayout->addWidget(mAddButton);

        // delete button
        mDeleteButton = new QPushButton("Remove monitor", this->parentWidget());
        monitorLayout->addWidget(mDeleteButton);

        // create and add a display widget
        mDisplayWidget = new ScreenDisplayWidget(this->parentWidget());
        mMainLayout->addWidget(mDisplayWidget);
    }

private slots:
    void onAddButton(){
        // parse resolution
        int horRes = mHorizontalResolutionInput->text().toInt();
        int verRes = mVerticalResolutionInput->text().toInt();

        // only allow monitors with non-zero area
        if(horRes == 0 || verRes == 0)
            return;

        bool added = mDisplayWidget->addMonitor(
                    mNameInput->text(),
                    horRes,
                    verRes);

        if(!added)
            QMessageBox::warning(this->parentWidget(), "Invalid name", "Monitor names must be unique", QMessageBox::Ok);
        else
            mNameInput->setText(mNameInput->text() + "x");
    }

    void onDeleteButton(){
        // retrieve monitor selection
        const QString& selected = mDisplayWidget->currentlySelectedMonitor();

        // abort if no monitor was selected
        if(selected == "HASHTAG_GOOD_CODING_NO_MONITOR_SELECTED")
            return;

        // show warning
        int del = QMessageBox::warning(
                    this->parentWidget(),
                    "Delete monitor",
                    "Do you really want to delete " + selected + "?",
                    QMessageBox::Yes,
                    QMessageBox::No | QMessageBox::Escape);

        // delete monitor if yes was selected
        if(del == QMessageBox::Yes)
            mDisplayWidget->deleteMonitor(selected);
    }

private:
    ScreenDisplayWidget* mDisplayWidget = nullptr;///< the custom widget used to display the monitor configuration
    QPushButton* mAddButton; ///< button to add a monitor
    QPushButton* mDeleteButton; ///< button to remove a monitor
    QLineEdit* mNameInput;
    QLineEdit* mVerticalResolutionInput;
    QLineEdit* mHorizontalResolutionInput;
    QVBoxLayout* mMainLayout;
};

#endif // SCREENCONFIGWIDGET_H

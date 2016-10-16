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
#include <QLabel>
#include <QFormLayout>

#include <assert.h>
#include <stdexcept>
#include <list>

namespace ScreenConfigWidget {

enum struct BorderIndex {
    BOTTOM = 0,
    RIGHT = 1,
    TOP = 2,
    LEFT = 3
};

template <typename T>
/**
 * @brief A helper struct for specifying screen dimensions
 */
struct Dimensions {
    /**
     * @brief Create a Dimensions struct with the specified values
     */
    Dimensions(T w, T h, T xOff = 0, T yOff = 0) : width(w), height(h), xOffset(xOff), yOffset(yOff) {
    }

    /// \brief create a zero-initialized Dimensions struct \overload
    Dimensions() {}

    T width = 0;///< object width
    T height = 0;///< object height
    T xOffset = 0;///< object horizontal offset
    T yOffset = 0;///< object vertical offset

    T left() const {
        return xOffset;
    }
    T xOff() const {
        return xOffset;
    }
    T top() const {
        return yOffset;
    }
    T yOff() const {
        return yOffset;
    }
    T right() const {
        return xOffset + width;
    }
    T bottom() const {
        return yOffset + height;
    }

    QRect qRect(double scale = 1) const {
        return QRect(QPoint(left(), top()) * scale, QSize(width, height) * scale);
    }
};

/// \brief Typedef for default geometry data type
typedef Dimensions<size_t> Geometry;

/**
 * @brief Specify the current mode:
 * ConfigureMonitors: add all monitors, and position them correctly
 * Select<X>Border: select the borders that belong to a border <X>
 */
enum struct InteractionMode {
    First_INVALID,
    ConfigureMonitors,
    SelectBottomBorder,
    SelectRightBorder,
    SelectTopBorder,
    SelectLeftBorder,
    Last_INVALID
};


/**
 * @brief Store the selection state and the geometry (scale 1) for each border
 */
struct Border {
    Geometry geometry;///< current border geometry (in pixels, scale 1)
    QColor drawColor = Qt::GlobalColor::lightGray;///< the color this border should be drawn in, default to grey

    /// \brief Create a (possibly scaled) QRect representation of this border for easy drawing
    QRect qRect(double scale = 1) const {
        QRect r = geometry.qRect(scale);

        if(r.width() < 2)
            r.setWidth(2);

        if(r.height() < 2)
            r.setHeight(2);

        return r;
    }
};

/*
 *
 *
 *
 *
 * *************************************************************************************************************************************************
 * Monitor
 * *************************************************************************************************************************************************
 *
 *
 *
 *
 */
struct Monitor {
    QRect boundingRectangle(double scale = 1.0) const {
        return QRect(top.qRect(scale).topLeft(), bottom.qRect(scale).bottomRight());
    }

    Monitor(const QString name, size_t width, size_t height,
            size_t xOffset, size_t yOffset,
            size_t letterboxOffsetX, size_t letterboxOffsetY) :
        mName(name),
        mWidth(width), mHeight(height),
        mXOffset(xOffset), mYOffset(yOffset),
        mVerticalLetterboxBarWidth(letterboxOffsetX), mHorizontalLetterboxBarHeight(letterboxOffsetY) {
        updateGeometry();
    }

    void updateGeometry() {
        left.geometry = Geometry(
                            BORDER_WIDTH, //width
                            mHeight - 2 * BORDER_WIDTH - (2 * mHorizontalLetterboxBarHeight), //height
                            mVerticalLetterboxBarWidth + mXOffset + 0, //x offset
                            mHorizontalLetterboxBarHeight + mYOffset + BORDER_WIDTH);// y offset

        right.geometry = Geometry(
                             BORDER_WIDTH, //width
                             mHeight - 2 * BORDER_WIDTH - (2 * mHorizontalLetterboxBarHeight), //height
                             (-mVerticalLetterboxBarWidth) + mXOffset + mWidth - BORDER_WIDTH, //x offset
                             mHorizontalLetterboxBarHeight + mYOffset + BORDER_WIDTH);// y offset

        top.geometry = Geometry(
                           mWidth - (2 * mVerticalLetterboxBarWidth), //width
                           BORDER_WIDTH, //height
                           mVerticalLetterboxBarWidth + mXOffset + 0, //x offset
                           mHorizontalLetterboxBarHeight + mYOffset + 0);// y offset

        bottom.geometry = Geometry(
                              mWidth - (2 * mVerticalLetterboxBarWidth), //width
                              BORDER_WIDTH, //height
                              mVerticalLetterboxBarWidth + mXOffset + 0, //x offset
                              (-mHorizontalLetterboxBarHeight) + mYOffset + mHeight - BORDER_WIDTH);// y offset
    }

    /**
     * @brief Retrieve border
     * @param i 0:bottom, 1:right, 2:top, 3:left
     */
    const Border& operator[] (size_t i) const {
        switch(i) {
        case 0:
            return bottom;
        case 1:
            return right;
        case 2:
            return top;
        case 3:
            return left;
        default:
            throw std::invalid_argument("index out of range 0-3");
        }
    }

    /**
     * @brief Retrieve border
     * @param i 0:bottom, 1:right, 2:top, 3:left
     */
    Border& operator[] (size_t i) {
        switch(i) {
        case 0:
            return bottom;
        case 1:
            return right;
        case 2:
            return top;
        case 3:
            return left;
        default:
            throw std::invalid_argument("index out of range 0-3");
        }
    }

    const QString& getName() const {
        return mName;
    }

    QString getName() {
        return mName;
    }

    void setPosition(const QPoint& targetPosition) {
        // calculate the delta (-> target - current) to the position of the monitor, so we can reuse move()
        move(targetPosition - boundingRectangle().topLeft());
    }

    void move(const QPoint& delta) {
        setXOffset(xOffset() + delta.x());
        setYOffset(yOffset() + delta.y());
    }

public:
    size_t width() const { return mWidth; } ///< screen geometry in pixels
    size_t height() const { return mHeight; } ///< screen geometry in pixels
    size_t xOffset() const { return mXOffset; } ///< screen offset in pixels
    size_t yOffset() const { return mYOffset; } ///< screen offset in pixels
    size_t verticalLetterboxBarWidth() const { return mVerticalLetterboxBarWidth; } ///< the height of the horizontal letterbox bars
    size_t horizontalLetterboxBarHeight() const { return mHorizontalLetterboxBarHeight; } ///< the width of the vertical letterbox bars

    void setWidth(size_t width) { mWidth = width; updateGeometry(); } ///< set screen geometry in pixels
    void setHeight(size_t height) { mHeight = height; updateGeometry(); } ///< set screen geometry in pixels
    void setXOffset(size_t xOff) { mXOffset = xOff; updateGeometry(); } ///< set screen offset in pixels
    void setYOffset(size_t yOff) { mYOffset = yOff; updateGeometry(); } ///< set screen offset in pixels
    void setVerticalLetterboxBarWidth(size_t vlbw) { mVerticalLetterboxBarWidth = vlbw; updateGeometry(); } ///< set the height of the horizontal letterbox bars
    void setHorizontalLetterboxBarHeight(size_t hlbw) { mHorizontalLetterboxBarHeight = hlbw; updateGeometry(); } ///< set the width of the vertical letterbox bars

private:
    QString mName;///< the identification of this monitor

    Border bottom, right, top, left;///< border geometry and selection state

    size_t mWidth;///< screen geometry in pixels
    size_t mHeight;///< screen geometry in pixels
    size_t mXOffset;///< screen offset in pixels
    size_t mYOffset;///< screen offset in pixels
    size_t mVerticalLetterboxBarWidth;///< the height of the horizontal letterbox bars
    size_t mHorizontalLetterboxBarHeight;///< the width of the vertical letterbox bars



    const size_t BORDER_WIDTH = 16;///< how wide each border should be
};


/*
 *
 *
 *
 *
 * *************************************************************************************************************************************************
 * SCREEN
 * *************************************************************************************************************************************************
 *
 *
 *
 *
 */
class Screen {
    std::list<Monitor> mMonitorList; ///< list of all the known monitors
    double mScale = 1.0 / 10.0;

    bool monitorExists(const QString& name) {
        bool exists = false;

        for(auto& m : mMonitorList)
            if(m.getName() == name)
                exists = true;

        return exists;
    }

    Monitor* getMonitor(const QString& name){
        for(auto& m : mMonitorList)
            if(m.getName() == name)
                return &m;
        return nullptr;
    }

    Monitor* mCurrentMonitorSelection = nullptr;

public:
    const Monitor* currentlySelectedMonitor() {
        return mCurrentMonitorSelection;
    }

    void drawText(QPainter& painter, const Monitor& m) {
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

    void drawBorders(QPainter& painter) {
        // draw all monitors
        for(const Monitor& monitor : mMonitorList) {
            // draw all borders
            for(size_t i = 0; i < 4; i++) {
                // draw a scaled down version of the borders
                painter.fillRect(monitor[i].qRect(mScale), monitor[i].drawColor);
            }

            // draw info text
            drawText(painter, monitor);
        }
    }

    void drawBoundingRectangle(QPainter& painter) {
        // draw all monitors
        for(const Monitor& monitor : mMonitorList) {
            QColor fillColor;
            if(&monitor == mCurrentMonitorSelection)
                fillColor = Qt::GlobalColor::darkGray;
            else
                fillColor = Qt::GlobalColor::lightGray;

            const QRect rect = monitor.boundingRectangle(mScale);

            // draw a scaled down version of the bounding rectangle
            painter.fillRect(rect, fillColor);

            // draw info text
            drawText(painter, monitor);
        }
    }

    void deleteMonitor(const QString& name) {
        mMonitorList.remove_if([name](Monitor& m) {
            return m.getName() == name;
        });

        // if we delete a monitor, the pointer may become invalid
        mCurrentMonitorSelection = nullptr;
    }

    bool addMonitor(const QString& name, int xRes, int yRes, int xOff = 0, int yOff = 0, int horLetterBox = 0, int verLetterBox = 0) {
        // allow unique names only
        if(monitorExists(name))
            return false;

        // if we add a monitor, the pointer may become invalid
        mCurrentMonitorSelection = nullptr;

        // add monitor
        mMonitorList.push_back(Monitor(name, xRes, yRes, xOff, yOff, horLetterBox, verLetterBox));

        // return true
        return true;
    }

    void moveMonitors(Monitor* mon, const QPoint& target, const QPoint& source, const QRect& bounding) {
        if(mon){
            mCurrentMonitorSelection = mon;
            snap(*mon, target, source, bounding);
        }
    }

    /**
     * @brief Snap monitor to points of interest instead of straight moving
     */
    void snap(Monitor& snapping, const QPoint& target, const QPoint& /*source*/, const QRect& masterBounding) {
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

        for(const Monitor& other : mMonitorList) {
            // we are only interested in the other monitors
            if(other.getName() == snapping.getName()) continue;
            QRect otherRect = other.boundingRectangle();
            // enlarge the other monitors rectangle to check for near collisions
            QRect otherTestRect = otherRect.adjusted(-widthTreshold / 2, -heightTreshold / 2, widthTreshold / 2, heightTreshold / 2);

            // check for collision...
            if(otherTestRect.intersects(snappingRectMoved)) {
                QRect intersection = otherTestRect.intersected(snappingRectMoved);
                // the intersection occured when moving right to left or vice versa
                if(intersection.height() > intersection.width()) {
                    // moving monitor is to the right
                    if(snappingRectMoved.right() > otherRect.right())
                        snappingRectMoved.moveLeft(otherRect.right() + 1);
                    // moving monitor is to the left
                    else if (snappingRectMoved.right() < otherRect.right())
                        snappingRectMoved.moveRight(otherRect.left() - 0);
                }
                // the intersection occured when moving top to down or vice versa
                else {
                    // moving monitor is to the top
                    if(snappingRectMoved.top() < otherRect.top())
                        snappingRectMoved.moveBottom(otherRect.top() - 0);
                    // moving monitor is to the bottom
                    else if (snappingRectMoved.top() > otherRect.top())
                        snappingRectMoved.moveTop(otherRect.bottom() + 1);
                }
            }
        }

        snapping.setPosition(snappingRectMoved.topLeft());
    }

    /**
     * @brief Toggle the selection state of a single monitor; returns true if the monitor is now selected
     */
    bool toggleSingleMonitorSelection(const QString& selection) {
        // if "selection" is already selected, clear the selection
        if(mCurrentMonitorSelection && mCurrentMonitorSelection->getName() == selection){
            mCurrentMonitorSelection = nullptr;
            return false;
        }
        else{
            mCurrentMonitorSelection = getMonitor(selection);
            return true;
        }
    }

    void deselectCurrent(){
        mCurrentMonitorSelection = nullptr;
    }

    Monitor* getMonitor(const QPoint &pos) {
        // find a clicked monitor
        for(Monitor& m : mMonitorList)
            if(m.boundingRectangle(mScale).contains(pos))
                return &m;
        return nullptr;
    }

    const QString getMonitorName(const QPoint& pos) const {
        // find a clicked monitor
        for(const Monitor& m : mMonitorList)
            if(m.boundingRectangle(mScale).contains(pos))
                return m.getName();
        return "";
    }

    /**
     * @brief Select a border: mainly specify the draw color
     * @param monitor which monitor does the border belong to
     * @param border border index
     * @param color new draw color
     */
    void selectBorder(const QString& monitor, const int border, QColor color) {
        // select only the monitor named like the selection
        for(Monitor& m : mMonitorList) {
            if(m.getName() == monitor) {
                m.operator [](border).drawColor = color;
            }
        }
    }

    /**
     * @brief Find if a border
     * @param pos click position
     * @param \out monitor name of the monitor the clicked border belongs to, or ""
     * @param border index of the border, or -1 \out
     */
    const Border* getBorder(const QPoint& pos, QString& monitor, int& border) const {
        monitor = "";
        border = -1;
        // select only the correctly named monitor
        for(const Monitor& m : mMonitorList) {
            // does the monitor contain the click (filter condition)
            if(m.boundingRectangle(mScale).contains(pos)) {
                // for each border
                for(int i = 0; i < 4; i++) {
                    // if border clicked
                    if(m[i].qRect(mScale).contains(pos)) {
                        monitor = m.getName();
                        border = i;
                        return &m[i];
                    }
                }
            }
        }
        return nullptr;
    }
};






/*
 *
 *
 *
 *
 * *************************************************************************************************************************************************
 * DISPLAY WIDGET
 * *************************************************************************************************************************************************
 *
 *
 *
 *
 */
class ScreenDisplayWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScreenDisplayWidget(QWidget *parent = 0) : QWidget(parent) {
        mScreen = new Screen();
    }

    const Monitor* currentlySelectedMonitor() {
        return mScreen->currentlySelectedMonitor();
    }

    void deleteMonitor(const QString& name) {
        mScreen->deleteMonitor(name);
        repaint();
    }

    bool addMonitor(const QString& name, int xRes, int yRes, int xOff = 0, int yOff = 0, int horLetterBox = 0, int verLetterBox = 0) {
        bool added = mScreen->addMonitor(name, xRes, yRes, xOff, yOff, horLetterBox, verLetterBox);
        repaint();
        return added;
    }

    void setInteractionMode(InteractionMode dm) {
        mInteractionMode = dm;
        repaint();
    }

    QVector<QVector<Border>> getResultingBorderConfiguration() {
        QVector<QVector<Border>> result(4);

        // copy all borders into the result vector vector
        for(int i = 0; i < 4; i++) {
            QVector<Border> bVec = result.at(i);
            for(const Border* b : mBorders[i])
                bVec.push_back(*b);
        }

        return result;
    }

    // drawing function
protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE {
        // create painter
        QPainter painter(this);

        // reset drawing area
        painter.fillRect(this->rect(), Qt::GlobalColor::white);

        switch(mInteractionMode) {
        case InteractionMode::ConfigureMonitors:
            mScreen->drawBoundingRectangle(painter);
            break;
        case InteractionMode::SelectBottomBorder:
        case InteractionMode::SelectRightBorder:
        case InteractionMode::SelectTopBorder:
        case InteractionMode::SelectLeftBorder:
            mScreen->drawBorders(painter);
            break;
        default:
            throw std::invalid_argument("unknown InteractionMode");
        }
    }

    // mouse signals
signals:
    void onMonitorSelected(Monitor* selection);
    void onMonitorDeSelected();
    void onMonitorMoved(Monitor* selection);

    // mouse handling functions
protected:
    void mousePressEvent(QMouseEvent *e) {
        if(mInteractionMode != InteractionMode::ConfigureMonitors)
            return;

        mLastMousePosition = e->pos();
        mClickedMonitor = mScreen->getMonitor(mLastMousePosition);
        mMouseMoved = false;
    }

    void mouseMoveEvent(QMouseEvent *e) {
        if(mInteractionMode != InteractionMode::ConfigureMonitors)
            return;

        mMouseMoved = true;
        mScreen->moveMonitors(mClickedMonitor, e->pos(), mLastMousePosition, this->rect());

        emit onMonitorMoved(mClickedMonitor);

        update();
    }

    /**
     * @brief Save the last mouse position, reset clicked monitor, and possibly select a monitor
     */
    void mouseReleaseEvent(QMouseEvent *e) {
        // save the last mouseposition
        mLastMousePosition = e->pos();

        // a monitor is no longer clicked
        mClickedMonitor = nullptr;

        // if the mouse did not move, this was a click event
        if(!mMouseMoved) {
            handleClick(e->pos());
        }
    }

    void handleClick(const QPoint& position) {
        if(mInteractionMode == InteractionMode::ConfigureMonitors) {
            // get clicked monitor
            Monitor* selected = mScreen->getMonitor(position);

            if(!selected){
                mScreen->deselectCurrent();
                emit onMonitorDeSelected();
            } else {
                // select clicked monitor
                bool selectionState =
                        mScreen->toggleSingleMonitorSelection(selected->getName());

                if(selectionState)
                    emit onMonitorSelected(selected);
                else
                    emit onMonitorDeSelected();
            }
        } else {
            // get clicked border
            QString selMonitor;
            int selBorderIndex;
            const Border* selBorder = mScreen->getBorder(position, selMonitor, selBorderIndex);

            // nothing clicked
            if(selBorderIndex < 0 || selMonitor == "")
                return;

            QColor selectionColor;

            // specify selection color
            switch(mInteractionMode) {
            case InteractionMode::SelectBottomBorder:
                selectionColor = Qt::GlobalColor::darkRed;
                break;
            case InteractionMode::SelectRightBorder:
                selectionColor = Qt::GlobalColor::darkBlue;
                break;
            case InteractionMode::SelectTopBorder:
                selectionColor = Qt::GlobalColor::darkGreen;
                break;
            case InteractionMode::SelectLeftBorder:
                selectionColor = Qt::GlobalColor::darkMagenta;
                break;
            default:
                assert(false && "unknown enum element");
            }

            // select clicked monitor
            if(selBorder->drawColor != selectionColor) {
                mScreen->selectBorder(selMonitor, selBorderIndex, selectionColor);
                mBorders[selBorderIndex].push_back(selBorder);
            }
            // unselect if the monitor was already selected
            else {
                mScreen->selectBorder(selMonitor, selBorderIndex, Qt::GlobalColor::lightGray);
                mBorders[selBorderIndex].removeAll(selBorder);
            }
        }
        // update screen
        update();
    }

    // mouse handling members
private:
    bool mMouseMoved = false;///< true if the mouse was moved since the last click
    Monitor* mClickedMonitor;///< last clicked monitor
    QPoint mLastMousePosition;

    // general members
private:
    InteractionMode mInteractionMode = InteractionMode::ConfigureMonitors;
    Screen* mScreen;

    // result members
private:
    QVector<const Border *> mBorders[4];
};





/*
 *
 *
 *
 *
 * *************************************************************************************************************************************************
 * TOP LEVEL WIDGET
 * *************************************************************************************************************************************************
 *
 *
 *
 *
 */
class ScreenConfigLayout : public QWidget {
    Q_OBJECT

public:
    explicit ScreenConfigLayout(QWidget *parent = 0) : QWidget(parent) {
        // create layout
        layout_();

        // connect to buttons
        connectSignals();

        // configure for initial mode
        configureForMode();
    }

    // main private slots, valid in all modes
private slots:
    void onNextModeButton() {
        // advance current mode
        mCurrentMode = static_cast<InteractionMode>(static_cast<int>(mCurrentMode) + 1);

        assert(mCurrentMode != InteractionMode::Last_INVALID);

        configureForMode();
    }

    void onPrevModeButton() {
        // advance current mode
        mCurrentMode = static_cast<InteractionMode>(static_cast<int>(mCurrentMode) - 1);

        assert(mCurrentMode != InteractionMode::First_INVALID);

        configureForMode();
    }

    // main member functions, valid in all modes
private:
    void configureForMode() {
        // disable invalid mode buttons
        mPrevModeButton->setEnabled((int) mCurrentMode != (((int) InteractionMode::First_INVALID) + 1));
        mNextModeButton->setEnabled((int) mCurrentMode != (((int) InteractionMode::Last_INVALID) - 1));

        // update display interaction mode
        mDisplayWidget->setInteractionMode(mCurrentMode);

        // hide/show screen config widgets
        mMonitorConfigurationWidget->setVisible(mCurrentMode == InteractionMode::ConfigureMonitors);

        switch(mCurrentMode) {
        case InteractionMode::ConfigureMonitors:
            mExplanationLabel->setText("Add and move screens as they are in your setup.");
            break;
        case InteractionMode::SelectBottomBorder:
            mExplanationLabel->setText("Select the borders belonging to the bottom border. <b>Important: you must keep a counter/clockwise order when selecting the borders throughout all steps!</b>");
            break;
        case InteractionMode::SelectRightBorder:
            mExplanationLabel->setText("Select the borders belonging to the right border. <b>Important: you must keep a counter/clockwise order when selecting the borders throughout all steps!</b>");
            break;
        case InteractionMode::SelectTopBorder:
            mExplanationLabel->setText("Select the borders belonging to the top border. <b>Important: you must keep a counter/clockwise order when selecting the borders throughout all steps!</b>");
            break;
        case InteractionMode::SelectLeftBorder:
            mExplanationLabel->setText("Select the borders belonging to the left border. <b>Important: you must keep a counter/clockwise order when selecting the borders throughout all steps!</b>");
            break;
        default:
            throw std::invalid_argument("unknown InteractionMode");
        }
    }

    // main member variables, valid in all modes
private:
    QHBoxLayout* mMainLayout;///< layout containing the screen widget and button layouts
    InteractionMode mCurrentMode = InteractionMode::ConfigureMonitors; ///< current interaction mode
    ScreenDisplayWidget* mDisplayWidget = nullptr;///< the custom widget used to display the monitor configuration
    QPushButton* mNextModeButton; ///< button to advance the selection mode
    QPushButton* mPrevModeButton; ///< button to un-advance the selection mode
    QLabel* mExplanationLabel;///< label for explanation

    // member variables for handling monitor config
private:
    QPushButton* mAddButton; ///< button to add a monitor
    QPushButton* mDeleteButton; ///< button to remove a monitor
    QLineEdit* mNameInput; ///< line edit for monitor names
    QLineEdit* mVerticalResolutionInput; ///< vertical resolution input
    QLineEdit* mHorizontalResolutionInput; ///< horizontal resolution input
    QLineEdit* mXOffInput; ///< x offset input
    QLineEdit* mYOffInput; ///< y offset input
    QLineEdit* mHorLetterboxInput; ///< horizontal letterboxing input
    QLineEdit* mVerLetterBoxInput; ///< vertical letterboxing input
    QWidget* mMonitorConfigurationWidget;
    Monitor* mLastSelectedMonitor = nullptr;

    // slots for handling monitor configuration
private slots:

    void onMonitorDeselected(){
        // if a monitor is selected, the add button will be disabled, vice versa with remove button
        mAddButton->setEnabled(true);
        mDeleteButton->setEnabled(false);

        mLastSelectedMonitor = nullptr;
    }

    void onMonitorSelected(Monitor* selection){
        // if a monitor is selected, the add button will be disabled, vice versa with remove button
        mAddButton->setEnabled(false);
        mDeleteButton->setEnabled(true);

        mLastSelectedMonitor = selection;

        readMonitorConfigToUi(mLastSelectedMonitor);
    }

    void readMonitorConfigToUi(Monitor* mon){
        if(!mon)
            return;

        mNameInput->setText(mon->getName());
        mHorizontalResolutionInput->setText(QString::number(mon->width()));
        mVerticalResolutionInput->setText(QString::number(mon->height()));
        mXOffInput->setText(QString::number(mon->xOffset()));
        mYOffInput->setText(QString::number(mon->yOffset()));
        mHorLetterboxInput->setText(QString::number(mon->horizontalLetterboxBarHeight()));
        mVerLetterBoxInput->setText(QString::number(mon->verticalLetterboxBarWidth()));
    }

    void updateCurrentMonitor(){
        if(!mLastSelectedMonitor)
            return;

        mLastSelectedMonitor->setWidth(mHorizontalResolutionInput->text().toInt());
        mLastSelectedMonitor->setHeight(mVerticalResolutionInput->text().toInt());
        mLastSelectedMonitor->setXOffset(mXOffInput->text().toInt());
        mLastSelectedMonitor->setYOffset(mYOffInput->text().toInt());
        mLastSelectedMonitor->setHorizontalLetterboxBarHeight(mHorLetterboxInput->text().toInt());
        mLastSelectedMonitor->setVerticalLetterboxBarWidth(mVerLetterBoxInput->text().toInt());
    }

    void onAddButton() {
        // parse resolution
        int horRes = mHorizontalResolutionInput->text().toInt();
        int verRes = mVerticalResolutionInput->text().toInt();
        int xOff = mXOffInput->text().toInt();
        int yOff = mYOffInput->text().toInt();
        int horLetterbox = mHorLetterboxInput->text().toInt();
        int verLetterbox = mVerLetterBoxInput->text().toInt();

        // only allow monitors with non-zero area
        if(horRes == 0 || verRes == 0)
            return;

        bool added = mDisplayWidget->addMonitor(
                         mNameInput->text(),
                         horRes, verRes,
                         xOff, yOff,
                         horLetterbox, verLetterbox);

        if(!added)
            QMessageBox::warning(this->parentWidget(), "Invalid name", "Monitor names must be unique", QMessageBox::Ok);
        else
            mNameInput->setText(mNameInput->text() + "x");
    }

    void onDeleteButton() {
        // retrieve monitor selection
        const Monitor* selected = mDisplayWidget->currentlySelectedMonitor();

        // removal should only be available when a monitor is selected
        assert(selected);

        // show warning
        int del = QMessageBox::warning(
                      this->parentWidget(),
                      "Delete monitor",
                      "Do you really want to delete " + selected->getName() + "?",
                      QMessageBox::Yes,
                      QMessageBox::No | QMessageBox::Escape);

        // delete monitor if yes was selected
        if(del == QMessageBox::Yes)
            mDisplayWidget->deleteMonitor(selected->getName());
    }

    // init member functions
private:
    void connectSignals() {
        // connect button click signals
        connect(mAddButton, SIGNAL(clicked()), this, SLOT(onAddButton()));
        connect(mDeleteButton, SIGNAL(clicked()), this, SLOT(onDeleteButton()));
        connect(mNextModeButton, SIGNAL(clicked()), this, SLOT(onNextModeButton()));
        connect(mPrevModeButton, SIGNAL(clicked()), this, SLOT(onPrevModeButton()));

        // when the monitor changes, update the ui
        connect(mDisplayWidget, SIGNAL(onMonitorSelected(Monitor*)), this, SLOT(onMonitorSelected(Monitor*)));
        connect(mDisplayWidget, SIGNAL(onMonitorDeSelected()), this, SLOT(onMonitorDeselected()));
        connect(mDisplayWidget, SIGNAL(onMonitorMoved(Monitor*)), this, SLOT(readMonitorConfigToUi(Monitor*)));

        // when the ui changes, update the monitor
        connect(mHorizontalResolutionInput, SIGNAL(textChanged(QString)), this, SLOT(updateCurrentMonitor()));
        connect(mVerticalResolutionInput, SIGNAL(textChanged(QString)), this, SLOT(updateCurrentMonitor()));
        connect(mXOffInput, SIGNAL(textChanged(QString)), this, SLOT(updateCurrentMonitor()));
        connect(mYOffInput, SIGNAL(textChanged(QString)), this, SLOT(updateCurrentMonitor()));
        connect(mHorLetterboxInput, SIGNAL(textChanged(QString)), this, SLOT(updateCurrentMonitor()));
        connect(mVerLetterBoxInput, SIGNAL(textChanged(QString)), this, SLOT(updateCurrentMonitor()));
    }

    void layoutMonitorConfig() {
        // monitor config layout
        mMonitorConfigurationWidget = new QWidget();
        QFormLayout* monitorConfigurationLayout = new QFormLayout(this->parentWidget());

        mMonitorConfigurationWidget->setLayout(monitorConfigurationLayout);

        mMainLayout->addWidget(mMonitorConfigurationWidget);

        // line edit widgets for entering resolution and name
        mNameInput = new QLineEdit("name");

        mHorizontalResolutionInput = new QLineEdit("1920");
        mVerticalResolutionInput = new QLineEdit("1080");

        mXOffInput = new QLineEdit("0");
        mYOffInput = new QLineEdit("0");

        mHorLetterboxInput = new QLineEdit("0");
        mVerLetterBoxInput = new QLineEdit("0");

        monitorConfigurationLayout->addRow(new QLabel("Name"), mNameInput);
        monitorConfigurationLayout->addRow(new QLabel("Horizontal Resolution"), mHorizontalResolutionInput);
        monitorConfigurationLayout->addRow(new QLabel("Vertical Resolution"), mVerticalResolutionInput);
        monitorConfigurationLayout->addRow(new QLabel("Horizontal Offset"), mXOffInput);
        monitorConfigurationLayout->addRow(new QLabel("Vertical Offset"), mYOffInput);
        monitorConfigurationLayout->addRow(new QLabel("Horizontal Letterboxing"), mHorLetterboxInput);
        monitorConfigurationLayout->addRow(new QLabel("Vertical Letterboxing"), mVerLetterBoxInput);

        // add button
        mAddButton = new QPushButton("Add screen");
        monitorConfigurationLayout->addRow(mAddButton);

        // delete button
        mDeleteButton = new QPushButton("Remove screen");
        mDeleteButton->setDisabled(true);
        monitorConfigurationLayout->addRow(mDeleteButton);
    }

    void layout_() {
        // set our main layout manager
        mMainLayout = new QHBoxLayout();
        setLayout(mMainLayout);

        layoutMonitorConfig();

        // add layout for mode button layout and instructions
        QVBoxLayout* controlLayout = new QVBoxLayout(this);
        mMainLayout->addLayout(controlLayout);

        // create and add a ScreenDisplayWidget
        mDisplayWidget = new ScreenDisplayWidget(this->parentWidget());
        controlLayout->addWidget(mDisplayWidget);

        mExplanationLabel = new QLabel(this);
        mExplanationLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        controlLayout->addWidget(mExplanationLabel);

        // add layout for mode buttons and instructions
        QHBoxLayout* buttonLayout = new QHBoxLayout(this->parentWidget());
        controlLayout->addLayout(buttonLayout);

        // prev mode button
        mPrevModeButton = new QPushButton("Prev mode", this->parentWidget());
        buttonLayout->addWidget(mPrevModeButton);

        // next mode button
        mNextModeButton = new QPushButton("Next mode", this->parentWidget());
        buttonLayout->addWidget(mNextModeButton);
    }
};
}// namespace screenconfigwidget
#endif // SCREENCONFIGWIDGET_H

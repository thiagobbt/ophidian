#include "mysfmlcanvas.h"

MySFMLCanvas::MySFMLCanvas(QWidget * parent) :
    QSFMLCanvas(parent),
    mCameraView(sf::FloatRect(0, 0, 51599.25, 34200.0))
{
    //mCameraView.setViewport(sf::FloatRect(0.0, 0.0, 1.0, 1.0));
    mCameraView.setSize(mCameraView.getSize().x, -mCameraView.getSize().y);
}

MySFMLCanvas::~MySFMLCanvas()
{
    delete mState;
}

void MySFMLCanvas::setController(MainController & controller)
{
    mMainController = &controller;
    mState = new Idle(this, mMainController);
}

void MySFMLCanvas::resizeEvent(QResizeEvent * e)
{
    sf::RenderWindow::create((sf::WindowHandle) winId());
}

void MySFMLCanvas::setSize(ophidian::geometry::Point size)
{
    mCameraView.setSize(size.x(), size.y());
}

void MySFMLCanvas::setState(State * state)
{
    mState = state;
}

Canvas * MySFMLCanvas::canvas()
{
    return &mCanvas;
}


void MySFMLCanvas::OnInit()
{

}

void MySFMLCanvas::OnUpdate()
{
    // mCanvas.update(); ???

    clear(sf::Color::Black);
    setView(mCameraView);

    draw(mCanvas);

    setView(getDefaultView());
    display();
}

void MySFMLCanvas::wheelEvent(QWheelEvent * e)
{
    if(e->delta() > 0)
        mCameraView.zoom(1.f/1.1f);
    else if(e->delta() < 0)
        mCameraView.zoom(1.1f);
}

void MySFMLCanvas::keyPressEvent(QKeyEvent * e)
{
    switch(e->key())
    {
    case Qt::Key::Key_W:
    case Qt::Key::Key_Up:
        mCameraView.move(sf::Vector2f(0.0f, -mCameraView.getSize().y*.1));
        break;
    case Qt::Key::Key_S:
    case Qt::Key::Key_Down:
        mCameraView.move(sf::Vector2f(0.0f, mCameraView.getSize().y*.1));
        break;
    case Qt::Key::Key_D:
    case Qt::Key::Key_Right:
        mCameraView.move(sf::Vector2f(mCameraView.getSize().x*.1, 0.0f));
        break;
    case Qt::Key::Key_A:
    case Qt::Key::Key_Left:
        mCameraView.move(sf::Vector2f(-mCameraView.getSize().x*.1, 0.0f));
        break;
    case Qt::Key::Key_Delete:
        mState->keyPressEvent(e);
    }
}

void MySFMLCanvas::mousePressEvent(QMouseEvent * e)
{
    mState->mousePressEvent(mouseEventToPoint(e));
}

void MySFMLCanvas::mouseMoveEvent(QMouseEvent * e)
{
    mState->mouseMoveEvent(mouseEventToPoint(e));
}

void MySFMLCanvas::mouseReleaseEvent(QMouseEvent * e)
{
    mState->mouseReleaseEvent(mouseEventToPoint(e));
}

bool MySFMLCanvas::findCellEvent(QString name)
{
    return mState->findCellEvent(name);
}

void MySFMLCanvas::centerViewOn(const ophidian::geometry::Point & p1)
{
    mCameraView.setCenter(sf::Vector2f(p1.x(), p1.y()));
}

void MySFMLCanvas::viewSize(const ophidian::geometry::Point & size)
{
    mCameraView.setSize(sf::Vector2f(size.x(), -size.y()));
}

void MySFMLCanvas::reserveMinimumOfQuads(std::size_t minimumOfQuads)
{
    mCanvas.reserveMinimumOfQuads(minimumOfQuads);
}

ophidian::geometry::Point MySFMLCanvas::mouseEventToPoint(QMouseEvent * e)
{
    sf::Vector2i pixelCoord{e->pos().x(), e->pos().y()};
    sf::Vector2f viewCoord{mapPixelToCoords(pixelCoord, mCameraView)};
    return ophidian::geometry::Point(viewCoord.x, viewCoord.y);
}

void MySFMLCanvas::updatePositionQuad(const ophidian::geometry::Point & p)
{
    mState->mouseReleaseEvent(p);
}

void MySFMLCanvas::saveToPNG(const std::string & filename)
{
    sf::RenderTexture texture;
    texture.create(1920, 1080);

    texture.setView(mCameraView);
    texture.clear(sf::Color::Black);
    texture.draw(mCanvas);
    texture.setView(texture.getDefaultView());

    std::cout << "Center: " << mCameraView.getCenter().x << " x " << mCameraView.getCenter().y << std::endl;
    std::cout << "Size: " << mCameraView.getSize().x << " x " << mCameraView.getSize().y << std::endl;
    std::cout << "Port: " << mCameraView.getViewport().width << " x " << mCameraView.getViewport().height << std::endl;

    texture.getTexture().copyToImage().saveToFile(filename);

    sf::Image upsideDownImage;
    upsideDownImage.loadFromFile(filename);
    upsideDownImage.flipVertically();
    upsideDownImage.saveToFile(filename);
}

void MySFMLCanvas::saveToSVG(const std::string & filename)
{
        double centerX = mCameraView.getCenter().x;
        double centerY = mCameraView.getCenter().y;
        double width = mCameraView.getSize().x;

        ophidian::geometry::Point min((centerX - width/2), (centerY - width/2));
        ophidian::geometry::Point max((centerX + width/2), (centerY + width/2));
        ophidian::geometry::Box viewSize(min, max);

        std::ofstream svg(filename);
        SVGMapper mapper(svg, width/100, width/100);
        ophidian::geometry::Box b(ophidian::geometry::Point(0,0), ophidian::geometry::Point(width/100,width/100));
        mapper.add(b);
        mapper.map(b, "fill:rgb(0,0,0);stroke:rgb(50,50,50)");

        mMainController->drawSVG(mapper, viewSize);
}

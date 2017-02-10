#include "gameboard.h"
#include "gridsquare.h"
#include "playareagridsquare.h"
#include "plane.h"

#include <QDebug>

GameBoard::GameBoard(PlaneGrid& pGrid, PlaneGrid& cGrid): m_PlayerGrid(pGrid), m_ComputerGrid(cGrid)
{
    m_Scene = new QGraphicsScene();
    m_View = new QGraphicsView(m_Scene);
}

void GameBoard::initializeBoardEditingItems()
{
    int rows = m_PlayerGrid.getRowNo() + 2 * m_PaddingEditingBoard;
    int cols = m_PlayerGrid.getColNo() + 2 * m_PaddingEditingBoard;


    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) {
            if (i < m_PaddingEditingBoard || abs(i - rows) <= m_PaddingEditingBoard
                    || j < m_PaddingEditingBoard || abs (j - cols) <= m_PaddingEditingBoard) {
                GridSquare* br = new GridSquare(i, j, m_SquareWidth);
                m_Scene->addItem(br);
                br->setPos(i * m_SquareWidth, j * m_SquareWidth);
                m_SceneItems[std::make_pair(i, j)] = br;
            } else {
                PlayAreaGridSquare* pabr = new PlayAreaGridSquare(i, j, m_SquareWidth);
                m_Scene->addItem(pabr);
                pabr->setPos(i * m_SquareWidth, j * m_SquareWidth);
                m_SceneItems[std::make_pair(i,j)] = pabr;
            }
        }
}


void GameBoard::showEditorBoard()
{
    initializeBoardEditingItems();
    displayPlayerPlanes();
}

///shows the planes on the grid
void GameBoard::displayComputerPlanes() {

}

///@todo: deal with overlapping planes
void GameBoard::displayPlayerPlanes() {
    for (int i = 0; i < m_PlayerGrid.getPlaneNo(); i++) {
        Plane pl;
        if (!m_PlayerGrid.getPlane(i, pl))
            continue;

        if (i != m_SelectedPlane)
            showPlane(pl);
        else
            showSelectedPlane(pl);
    }
}

void GameBoard::hidePlayerPlanes()
{
    int rows = m_PlayerGrid.getRowNo() + 2 * m_PaddingEditingBoard;
    int cols = m_PlayerGrid.getColNo() + 2 * m_PaddingEditingBoard;


    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            m_SceneItems[std::make_pair(i, j)]->clearPlaneOptions();
}

///shows the computer guess on the grid
void GameBoard::displayComputerGuesses() {

}

///shows the player guess on the grid
void GameBoard::displayPlayerGuesses() {

}

void GameBoard::showPlane(const Plane &pl)
{
    QPoint head = pl.head();
    m_SceneItems[std::make_pair(head.x() + m_PaddingEditingBoard, head.y() + m_PaddingEditingBoard)]->setType(GridSquare::Type::PlaneHead);
    PlanePointIterator ppi(pl);
    ///ignore the plane head
    ppi.next();
    while (ppi.hasNext()) {
        QPoint pt = ppi.next();
        m_SceneItems[std::make_pair(pt.x() + m_PaddingEditingBoard, pt.y() + m_PaddingEditingBoard)]->setType(GridSquare::Type::Plane);
    }
}

void GameBoard::showSelectedPlane(const Plane &pl)
{
    QPoint head = pl.head();
    m_SceneItems[std::make_pair(head.x() + m_PaddingEditingBoard, head.y() + m_PaddingEditingBoard)]->setSelected(true);
    PlanePointIterator ppi(pl);
    ///ignore the plane head
    ppi.next();
    while (ppi.hasNext()) {
        QPoint pt = ppi.next();
        m_SceneItems[std::make_pair(pt.x() + m_PaddingEditingBoard, pt.y() + m_PaddingEditingBoard)]->setSelected(true);
    }
}

void GameBoard::selectPlaneClicked(bool)
{
    m_SelectedPlane = (m_SelectedPlane + 1) % m_PlayerGrid.getPlaneNo();
    qDebug() << "select plane " << m_SelectedPlane;
    hidePlayerPlanes();
    displayPlayerPlanes();
}

void GameBoard::rotatePlaneClicked(bool)
{
    qDebug() << "rotate plane";
    m_PlayerGrid.rotatePlane(m_SelectedPlane);
    hidePlayerPlanes();
    displayPlayerPlanes();
}

void GameBoard::upPlaneClicked(bool )
{
    qDebug() << "up plane";
    m_PlayerGrid.movePlaneUpwards(m_SelectedPlane);
    hidePlayerPlanes();
    displayPlayerPlanes();
}

void GameBoard::downPlaneClicked(bool )
{
    qDebug() << "down plane";
    m_PlayerGrid.movePlaneDownwards(m_SelectedPlane);
    hidePlayerPlanes();
    displayPlayerPlanes();
}

void GameBoard::leftPlaneClicked(bool )
{
    qDebug() << "left plane";
    m_PlayerGrid.movePlaneLeft(m_SelectedPlane);
    hidePlayerPlanes();
    displayPlayerPlanes();
}

void GameBoard::rightPlaneClicked(bool )
{
    qDebug() << "right plane";
    m_PlayerGrid.movePlaneRight(m_SelectedPlane);
    hidePlayerPlanes();
    displayPlayerPlanes();
}

void GameBoard::doneClicked(bool )
{
    qDebug() << "done";
}

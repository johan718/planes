#include "computerlogic.h"
#include <QDebug>
#include <algorithm>
#include <QPoint>

//default constructor
PlaneOrientationData::PlaneOrientationData()
{
    m_plane = Plane(0, 0, (Plane::Orientation)0);
    m_discarded = true;
    m_pointsNotTested.clear();
}

//useful constructor
PlaneOrientationData::PlaneOrientationData(const Plane& pl, bool isDiscarded):
    m_plane(pl),
    m_discarded(isDiscarded)
{
    PlanePointIterator ppi(m_plane);

    //all points of the plane besides the head are not tested yet
    ppi.next();

    while(ppi.hasNext())
    {
        m_pointsNotTested.append(ppi.next());
    }
}

//copy constructor
PlaneOrientationData::PlaneOrientationData(const PlaneOrientationData& pod):
    m_plane(pod.m_plane),
    m_discarded(pod.m_discarded),
    m_pointsNotTested(pod.m_pointsNotTested) {
}
//assignment operator
void PlaneOrientationData::operator=(const PlaneOrientationData &pod)
{
    m_plane = pod.m_plane;
    m_discarded = pod.m_discarded;
    m_pointsNotTested = pod.m_pointsNotTested;
}

void PlaneOrientationData::update(const GuessPoint &gp)
{
    //if plane is discarded return
    if(m_discarded)
        return;

    //find the guess point in the list of points not tested
    int idx = m_pointsNotTested.indexOf(QPoint(gp.m_row, gp.m_col));

    //if point not found return
    if(idx == -1)
        return;

    //if point found
    //if dead and idx = 0 remove the head from the list of untested points
    if(gp.m_type == GuessPoint::Dead && idx == 0)
    {
        m_pointsNotTested.removeAt(idx);
        return ;
    }

    //if miss or dead discard plane
    if(gp.m_type == GuessPoint::Miss || gp.m_type == GuessPoint::Dead)
        m_discarded = true;

    //if hit take point out of the list of points not tested
    if(gp.m_type == GuessPoint::Hit)
        m_pointsNotTested.removeAt(idx);
}

//checks to see that all points on the plane were tested
bool PlaneOrientationData::areAllPointsChecked()
{
    return (m_pointsNotTested.size() == 0);
}

//constructs the head data structure
//m_row and m_col give the size of the grid
//m_headRow and m_headCol
//are the positions of the planes head
//m_correctOrient is the orientation of the plane
HeadData::HeadData(int row, int col, int headRow, int headCol):
    m_row(row),
    m_col(col),
    m_headRow(headRow),
    m_headCol(headCol),
    m_correctOrient(-1)
{

    for(int i = 0;i < 4; i++)
    {
        Plane pl(m_headRow, m_headCol, (Plane::Orientation)i);
        //create the four planes for each head position
        m_options[i] = PlaneOrientationData(pl, false);

        if(!pl.isPositionValid(m_row, m_col))
            m_options[i].m_discarded = true;
    }
    //check which of the four possible orientation is a valid position
    //for the invalid positions mark that orientation as discarded
}

//update a head data structure with a guess
bool HeadData::update(const GuessPoint& gp)
{
    //if the head data is already conclusive ignore
    if(m_correctOrient != -1)
        return true;

    //update the four plane positions with this new data
    for(int i = 0;i < 4; i++)
        m_options[i].update(gp);

    //verify if we checked all points of a plane
    for(int i = 0;i < 4; i++)
    {
        if(!m_options[i].m_discarded && m_options[i].areAllPointsChecked())
        {
            m_correctOrient = i;
            return true;
        }
    }

    //verify if 3 of the 4 possible orientations are discarded
    int count = 0;
    int good_orientation = -1;
    for(int i = 0;i < 4; i++)
    {
        if(m_options[i].m_discarded)
            count++;
        else
            good_orientation = i;
    }
    //if there are exactly 3 wrong orientations
    if(count == 3)
    {
        m_correctOrient = good_orientation;
        return true;
    }
    return false;
}

//constructs a computer logic object
//m_row, m_col give the size of the grid
//maxChoiceNo is the number of possible plane positions on the grid
//m_PlaneNo are the number of planes that need to be guessed
ComputerLogic::ComputerLogic(int row, int col, int planeno):
    m_row(row),
    m_col(col),
    maxChoiceNo(row * col * 4),
    m_planeNo(planeno)
{
    //creates the tables of choices
    m_choices = new int[maxChoiceNo];
    m_zero_choices = new int[maxChoiceNo/4];

    //initializes the table of choices and the head data
    reset();

    //initializes the iterator that generates all the planes that pass
    //through (0,0)
    m_pipi.reset();
}

//selects all the possible plane positions that are valid within the given grid
void ComputerLogic::reset()
{
    //initializes -1 for impossible choice (invalid plane position)
    //with 0 for possible choice
    for(int i = 0;i < maxChoiceNo; i++)
    {
        Plane pl = mapIndexToPlane(i);
        if(pl.isPositionValid(m_row, m_col))
            m_choices[i] = 0;
        else
            m_choices[i] = -1;
    }

    //clears various lists in the computerlogic object
    m_guessedPlaneList.clear();
    //m_guessedHeadList.clear();
    m_guessesList.clear();
    m_extendedGuessesList.clear();
    m_headDataList.clear();
}

//destructor
ComputerLogic::~ComputerLogic()
{
    //deletes the object containing the choices
    delete [] m_choices;
    delete [] m_zero_choices;
}

//computes the position in the m_choices array of a given plane
int ComputerLogic::mapPlaneToIndex(const Plane& pl) const
{
    int temp = (pl.col() * m_row + pl.row()) * 4 + (int)pl.orientation();
    return temp;
}

//computes the plane corresponding to a given position in the choices array
Plane ComputerLogic::mapIndexToPlane(int idx) const
{
    Plane::Orientation o = (Plane::Orientation)(idx % 4);
    int temp = idx / 4;

    int row = temp % m_row;
    int col = temp / m_row;

    return Plane(row, col, o);
}

//computes the plane head position corresponding for a given position in the choices array
QPoint ComputerLogic::mapIndexToQPoint(int idx) const
{
    int temp = idx / 4;

    int row = temp % m_row;
    int col = temp / m_row;

    return QPoint(row, col);
}

//chooses the next point
bool ComputerLogic::makeChoice(QPoint& qp) const
{
    //based on the 3 strategies of choice choses 3 possible moves
    QPoint qp1, qp2, qp3;

    bool test1 = makeChoiceFindHeadMode(qp1);
    bool test2 = makeChoiceFindPositionMode(qp2);
    bool test3 = makeChoiceRandomMode(qp3);

    //if there are no more choices to be tested return false
    if(!test1)
        return false;

    //generates a random number smaller than 10
    int idx = Plane::generateRandomNumber(10);

    //various random strategies for making a choice
    if(test2 && test3) {
        if(idx<6) {
            qp = qp1;
            return true;
        }

        if(idx<9) {
            qp = qp2;
            return true;
        }

        qp = qp3;
        return true;
    }

    if(!test2 && test3) {
        if(idx<7) {
            qp = qp1;
            return true;
        }

        qp = qp3;
        return true;
    }

    if(test2 && !test3) {
        if(idx<7) {
            qp = qp1;
            return true;
        }

        qp = qp2;
        return true;
    }

    if(!test2 && !test3) {
        qp = qp1;
        return true;
    }

    return false;
}


//choses the most likely point to be a head's plane on the players grid

bool ComputerLogic::makeChoiceFindHeadMode(QPoint& qp) const
{
    QList<int> maxPos;

    //computes the point on the m_choices table
    //which has the highest value
    int maxidx = 0;
    maxPos.clear();

    for(int i = 1;i < maxChoiceNo; i++)
    {
        if(m_choices[i] == m_choices[maxidx])
            maxPos.append(i);

        if(m_choices[i] > m_choices[maxidx])
        {
            maxidx = i;
            maxPos.clear();
            maxPos.append(i);
        }
    }

    //if all the choices are impossible returns false
    if(m_choices[maxidx] == -1)
        return false;

    //choses randomly a point with the maximum probability
    int idx = Plane::generateRandomNumber(maxPos.size());

    //converts the choice into a plane's head position
     qp=mapIndexToQPoint(maxPos.at(idx));
    return true;
}


//after finding one or more heads the computer tries to
//determine the real position of the found plane
bool ComputerLogic::makeChoiceFindPositionMode(QPoint& qp) const
{
    //chose randomly a head data from the list
    //and choose randomly an orientation which is not discarded
    //select a point which was not selected from this orientation

    //if there are no head data structures return false
    if(m_headDataList.size() == 0)
        return false;

    //choses a random plane head from the list of heads
    int idx = Plane::generateRandomNumber(m_headDataList.size());
    HeadData hd=m_headDataList.at(idx);

    //find the orientation that has the most not tested points
    //and is not discarded

    int max_not_tested = 0;
    int good_orientation = -1;
    for(int i = 0;i < 4; i++)
    {
        PlaneOrientationData pod = hd.m_options[i];

        if(!pod.m_discarded) {
            if(pod.m_pointsNotTested.size()>max_not_tested) {
                max_not_tested = pod.m_pointsNotTested.size();
                good_orientation = i;
            }
        }
    }

    //if there is no not discarded position with more than zero points not tested
    //return false
    if(good_orientation == -1)
        return false;

    //choose randomly a point from the points not tested in the chosen orientation
    idx = Plane::generateRandomNumber(hd.m_options[good_orientation].m_pointsNotTested.size());

    qp = hd.m_options[good_orientation].m_pointsNotTested.at(idx);

    return true;
}

//computer choses a point about which has no
//positive or negative data
bool ComputerLogic::makeChoiceRandomMode(QPoint& qp) const
{
    //find a random point which has zero score in the choice map
    int idx = Plane::generateRandomNumber(maxChoiceNo);

    //starting from the point next to the point selected
    int count = (idx+1) % maxChoiceNo;

   //if it corresponds to a point with a choice of 0
    //choose this point
    while(count != idx)
    {
        if(m_choices[count] == 0) {
            qp = mapIndexToQPoint(count);
            return true;
        }
        //if the point does not correspond to a zero choice
        //move to the next point
        count = (count+1) % maxChoiceNo;
    }
    //loop until all the points in the m_choices table have been tested

    return false;
}

//checks whether if the computer has guessed everything
bool ComputerLogic::areAllGuessed() const
{
    return (m_guessedPlaneList.size() >= m_planeNo);
}

//new info is added the choices are updated
//we assume that choices do not repeat themselves
//for safety we should keep a list of guesses and keep
//checking if they repeat
void ComputerLogic::addData(const GuessPoint& gp)
{
    //add to list of guesses
    m_guessesList.append(gp);
    m_extendedGuessesList.append(gp);

    //updates the info in the array of choices
    updateChoiceMap(gp);

    //updates the head data
    updateHeadData(gp);

    //checks all head data to see if any plane positions were confirmed
    QMutableListIterator<HeadData> qmli(m_headDataList);

    while(qmli.hasNext()) {
        HeadData hd = qmli.next();

        //if we decided upon an orientation
        //update the choice map
        //and delete the head data structure
        //append to the list of found planes
        if(hd.m_correctOrient != -1)
        {
            Plane pl(hd.m_headRow, hd.m_headCol, (Plane::Orientation)hd.m_correctOrient);
            updateChoiceMapPlaneData(pl);
            m_guessedPlaneList.append(pl);
            qmli.remove();
        }
    }

}

//updates the computer choices
void ComputerLogic::updateChoiceMap(const GuessPoint& gp) {

    //marks all the 4 positions in the choice map as guessed -2
    for(int i = 0;i < 4; i++) {
        Plane plane(gp.m_row, gp.m_col, (Plane::Orientation)i);
        int idx = mapPlaneToIndex(plane);
        m_choices[idx] = -2;
    }

    if(gp.m_type == GuessPoint::Dead)
        updateChoiceMapDeadInfo(gp.m_row, gp.m_col);

    if(gp.m_type == GuessPoint::Hit)
        updateChoiceMapHitInfo(gp.m_row, gp.m_col);

    if(gp.m_type == GuessPoint::Miss)
        updateChoiceMapMissInfo(gp.m_row, gp.m_col);
}

void ComputerLogic::updateChoiceMapPlaneData(const Plane& pl)
{
    //interprets a plane as a list of miss guesses
    //updates the choice map with this list of guesses
    //and appends the guesses to the list of guesses
    PlanePointIterator ppi(pl);

    //not to treat the head of the plane
    ppi.next();

    while(ppi.hasNext()) {
        QPoint qp = ppi.next();
        GuessPoint gp(qp.x(), qp.y(), GuessPoint::Miss);
        updateChoiceMap(gp);
        m_extendedGuessesList.removeOne(gp);
        m_extendedGuessesList.append(gp);
    }
}

//updates the choices with info about a dead guess
void ComputerLogic::updateChoiceMapDeadInfo(int row, int col)
{
    //do nothing as everything is done in the updateHeadData function
    //the decision to chose a plane is made in the
    //updateHeadData function
    updateChoiceMapMissInfo(row, col);
}

//updates the choices with info about a hit guess
void ComputerLogic::updateChoiceMapHitInfo(int row,int col)
{
    //for all the plane positions that are valid and that contain the
    //current position increment their score

    m_pipi.reset();

    while(m_pipi.hasNext()) {
        //obtain index for position that includes QPoint(row,col)
        Plane pl = m_pipi.next();
        QPoint qp(row, col);
        //add current position to the index to obtain a plane option
        pl = pl + qp;

        //if choice is not valid continue to the next position
        if(!pl.isPositionValid(m_row, m_col))
            continue;

        //position is valid; check first that it has not
        //being marked as invalid and that increase its score

        int idx = mapPlaneToIndex(pl);
        if(m_choices[idx] >= 0)
            m_choices[idx]++;
    }
}

//updates the choices with info about a miss guess
void ComputerLogic::updateChoiceMapMissInfo(int row, int col)
{

    //discard all plane positions that contain this point
    m_pipi.reset();

    while(m_pipi.hasNext())
    {
        //obtain index for position that includes QPoint(row,col)
        Plane pl = m_pipi.next();
        QPoint qp(row, col);
        //add current position to the index to obtain a plane option
        pl = pl + qp;

        //if choice is not valid continue to the next position
        if(!pl.isPositionValid(m_row, m_col))
            continue;

        //position is valid; because it includes a miss
        //it must be taken out from the list of choice

        int idx = mapPlaneToIndex(pl);
        if(m_choices[idx] >= 0)
            m_choices[idx] = -1;
    }
}

//updates the head data with a new guess
void ComputerLogic::updateHeadData(const GuessPoint& gp)
{
    //build a list iterator that allows the modification of data
    QMutableListIterator<HeadData> qmli(m_headDataList);

    //updates the head data with the found guess point
    while(qmli.hasNext()) {
        HeadData hd = qmli.next();
        hd.update(gp);
        qmli.setValue(hd);
    }

    //if the guess point is a head  add a new head data
    //which contains all the knowledge gathered until now
    if(gp.isDead())
    {
        //create a new head data structure
        HeadData hd(m_row, m_col, gp.m_row, gp.m_col);

        //update the head data with all the history of guesses
        for(int i = 0;i < m_extendedGuessesList.size(); i++)
            hd.update(m_extendedGuessesList.at(i));

        //append the head data in the list of heads
        m_headDataList.append(hd);
    }
}

//Calculate the number of choice points influenced by a point
int ComputerLogic::noPointsInfluenced(const QPoint& qp)
{
    //checks to see if the point belongs already to a guess
    //or if it cannot be a viable choice
    //when this happens returns -1
    bool point_not_good = true;

    for(int i = 0;i < 4; i++)
    {
        Plane pl(qp, (Plane::Orientation)i);
        int idx = mapPlaneToIndex(pl);
        if(m_choices[idx] >= 0) {
            point_not_good = false;
            break;
        }
    }

    if(point_not_good)
        return -1;

    int count = 0;

    //get the planes intersecting the point
    PlaneIntersectingPointIterator pipi(qp);

    while(pipi.hasNext()) {
        //for each plane
        Plane pl = pipi.next();
        //ignore if it's head is in the initial point
        if(pl.head() == qp)
            continue;

        //find the index of the point
        int idx = mapPlaneToIndex(pl);

        if(m_choices[idx] >= 0)
            count++;
    }

    return count;
}

//equals operator: copies the choices array and the guesses list from the ComputerLogic object
void RevertComputerLogic::operator=(const ComputerLogic& cl)
{
    if(m_row!=cl.getRowNo() && m_col!=cl.getRowNo())
        return;

    const int* choices = cl.getChoicesArray();

    for(int i = 0;i < maxChoiceNo; i++)
        m_choices[i] = choices[i];

    m_guessesList.clear();
    m_guessesList = cl.getListGuesses();

    m_extendedGuessesList.clear();
    m_extendedGuessesList = cl.getExtendedListGuesses();

    m_playList.clear();
    m_playList = cl.getListGuesses();

    m_pos = m_playList.size()-1;
}

//constructor
RevertComputerLogic::RevertComputerLogic(int row, int col, int planeno):
    ComputerLogic(row, col, planeno), m_pos(0) {
    m_playList.clear();
}


//reverts the computer strategy by n steps
void RevertComputerLogic::revert(int n)
{
    if(n <= 0)
        return;

    //computes to which step of the computer strategy the strategy must be reverted
    int nsteps = m_pos-n;

    if(nsteps < -1)
        nsteps = -1;

    //resets the computer logic object
    reset();

    //play the strategy forwards
    for(int i = 0;i <= nsteps; i++)
    {
        addData(m_playList.at(i));
    }
    m_pos = nsteps;
}

//plays the computer strategy forward
void RevertComputerLogic::next()
{
    if(m_pos >= m_playList.size() - 1)
        return;

    addData(m_playList.at(m_pos + 1));
    m_pos++;
}


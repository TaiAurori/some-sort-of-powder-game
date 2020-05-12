// powdergame-early.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cstdlib>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>

// does nothing; placeholder for pixels with empty update function
void noOperation() {};

// class for pixels 
class Pixel {
    public:
        //default pixel constructor, results in blank pixel (air)
        Pixel();
        //regular pixel constructor, asks for the pixel's name, if it falls, and it's color
        Pixel (std::string, bool, sf::Color);
        //pixel's name (to be used for pixel selection menu)
        std::string name = "Air";
        //should the pixel be acknowledged and updated? if so, this is true (used for air mostly)
        bool exists = false;
        //is the pixel affected by gravity?
        bool can_fall = false;
        //pixel's color in SFML color format
        sf::Color color = sf::Color::Black;
        //pixel's specific update function
        void(*update) = noOperation;
        //how long until the pixel falls, and the default fall timer (used to reset current fall timer)
        int fall_timer_current = 10;
        int fall_timer_default = 100;
};

//regular constructor for pixel
Pixel::Pixel(std::string name_given, bool can_fall_given, sf::Color color_given) {
    name = name_given;
    exists = true;
    can_fall = can_fall_given;
    color = color_given;
}

//default constructor (results in blank pixel)
Pixel::Pixel() {
    name = "Air";
    exists = false;
    can_fall = false;
    color = sf::Color::Black;
    void(*update) = noOperation;
    fall_timer_current = 2;
    fall_timer_default = 2;
}

int main()
{
    // the size of each individual powder pixel
    const int pixelsize{ 5 };
    float falltimerdecrementfactor = 0.15;

    // window sizes
    const int windowsizeX{ 600 };
    const int windowsizeY{ 600 };

    // init window and set framerate limit
    sf::RenderWindow window(sf::VideoMode(windowsizeX, windowsizeY), "Pixel Game", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    //try {
    const int pssizeX{ windowsizeX / pixelsize };
    const int pssizeY{ windowsizeY / pixelsize };

    // 2d array of pixels as pixel classes
    Pixel pixelspace[pssizeX][pssizeY];

    //these are the pixels the user can select
    Pixel pixelentries[] = {
        Pixel("Generic (falling)", true, sf::Color::White),
        Pixel("Generic (stationary)", false, sf::Color::White),
        //Pixel()
    };
    //the selected pixel that the user will place when clicking
    Pixel selectedPixel = pixelentries[0];

    //the index of the selected pixel in pixelentries (to be made a negative number if selected pixel has no valid index)
    int selectedPixelIndex = 0;

    //the size of the pixel array because, once again, c++ is bad
    int pixelEntrySize = 2;

    //init arialFont and load arial.ttf from the windows fonts dir into it (better solution to be deduced later)
    sf::Font arialFont;
    arialFont.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");

    // sets up the "selected element" text
    sf::Text selectedPixelText;
    selectedPixelText.setString("selected: " + pixelentries[selectedPixelIndex].name);
    selectedPixelText.setFillColor(sf::Color::White);
    selectedPixelText.setOutlineColor(sf::Color::Black);
    selectedPixelText.setFont(arialFont);
    selectedPixelText.setCharacterSize(24);

    // queue for pixel operations (only fall operations currently)
    sf::Vector2i pixelfallqueue[pssizeX * pssizeY];
    int pfq_size = 0;

    // mouse wheel event set on every mouse wheel event
    sf::Event::MouseWheelEvent mouseWheel;
    //vertical "total position" of the mouse wheel
    int mouseWheelPos = 0;
    //has the mouse wheel position been changed last frame?
    bool mouseWheelChanged = false;
    //the amount of ticks before the element changes
    int mouseWheelSensitivity = 6;
    //if this is disabled, piling and other computationally heavy activities will NOT be performed.
    bool advancedPhysics = true;

    //the pixel being dealt with in the main loop
    Pixel pixel;

    //the rectangle shape that will be used to draw every pixel
    sf::RectangleShape pixelrect(sf::Vector2f(pixelsize, pixelsize));
    pixelrect.setFillColor(sf::Color::White);

    //the rectangle that will be used to draw the pixel placing preview (try saying that 5 times fast)
    sf::RectangleShape mouserect(sf::Vector2f(pixelsize, pixelsize));

    //pixel preview's color
    sf::Color mouseprevcolor;

    // while the window is open, do this
    while (window.isOpen())
    {
        // grab events from the event queue
        sf::Event event;
        while (window.pollEvent(event))
        {
            // if the X on the window was clicked, close the window
            if (event.type == sf::Event::Closed)
                window.close();
            // if mouse wheel scrolled, update mouse wheel variable outside
            if (event.type == sf::Event::MouseWheelMoved) {
                mouseWheel = event.mouseWheel;
                mouseWheelChanged = true;
            };
        }

        window.clear(sf::Color::Black);

        //position of pixel in array
        sf::Vector2i pos;

        //for each pixel in the fall queue, do...
        for (int i = 0; i < pfq_size; i++) {
            //set position to the pixel's position stored in pixelfallqueue
            pos = pixelfallqueue[i];
            //if the pixel is on the ground, skip over this iteration
            if (pos.y == 0) continue;

            //if there is a free space under this pixel, then...
            if (!pixelspace[pos.x][pos.y - 1].exists) {
                //set the pixel underneath to the current pixel
                pixelspace[pos.x][pos.y - 1] = pixelspace[pos.x][pos.y];
                //set the current pixel to a blank pixel
                pixelspace[pos.x][pos.y] = Pixel();
            }
            // if there is no free space directly under this pixel, then check if advancedPhysics is enabled. if so, then...
            else if (advancedPhysics) {
                // if there is a space available on either side and down (down left diagonally, left, down right diagonally, right are free), then...
                if (!pixelspace[pos.x - 1][pos.y - 1].exists && !pixelspace[pos.x + 1][pos.y - 1].exists) {
                    // if the pixels above both the free spaces are not occupied, then...
                    if (!pixelspace[pos.x - 1][pos.y].exists && !pixelspace[pos.x + 1][pos.y].exists) {
                        // take a random number and modulate by 2. if the number is even, then...
                        if (rand() % 2 == 0) {
                            //move pixel left and down
                            pixelspace[pos.x - 1][pos.y - 1] = pixelspace[pos.x][pos.y];
                        }
                        else {
                            //move pixel right and down
                            pixelspace[pos.x + 1][pos.y - 1] = pixelspace[pos.x][pos.y];
                        }
                        // make the current pixel space empty
                        pixelspace[pos.x][pos.y] = Pixel();
                    }
                    //if only the left space has no pixel on or below it, then move left and down
                    else if (!pixelspace[pos.x - 1][pos.y].exists) {
                        pixelspace[pos.x - 1][pos.y - 1] = pixelspace[pos.x][pos.y];
                        pixelspace[pos.x][pos.y] = Pixel();
                    }
                    //if only the right space has no pixel on or below it, then move right and down
                    else if (!pixelspace[pos.x + 1][pos.y].exists) {
                        pixelspace[pos.x + 1][pos.y - 1] = pixelspace[pos.x][pos.y];
                        pixelspace[pos.x][pos.y] = Pixel();
                    }
                }
                // if the left space is available and has no pixel on or below it, then move that way
                else if (!pixelspace[pos.x - 1][pos.y - 1].exists && !pixelspace[pos.x - 1][pos.y].exists) {
                    pixelspace[pos.x - 1][pos.y - 1] = pixelspace[pos.x][pos.y];
                    pixelspace[pos.x][pos.y] = Pixel();
                }
                // if the right space is available and has no pixel on or below it, then move that way
                else if (!pixelspace[pos.x + 1][pos.y - 1].exists && !pixelspace[pos.x + 1][pos.y].exists) {
                    pixelspace[pos.x + 1][pos.y - 1] = pixelspace[pos.x][pos.y];
                    pixelspace[pos.x][pos.y] = Pixel();
                }
            }
            //reset fall timer
            pixelspace[pos.x][pos.y].fall_timer_current = pixelspace[pos.x][pos.y].fall_timer_default;
        }
        //reset pixel fall queue size
        pfq_size = 0;

        //iteration values, matching mathematical position values
        int x;
        int y;

        //loop over every pixel and update accordingly
        for (x = 0; x < pssizeX - 1; x++) {
            for (y = 0; y < pssizeY - 1; y++) {
                if (pixelspace[x][y].exists) {
                    //if the pixel can fall, do this
                    if (pixelspace[x][y].can_fall) {
                        //decrement the fall timer
                        pixelspace[x][y].fall_timer_current -= pixelspace[x][y].fall_timer_default * (pixelsize * falltimerdecrementfactor);

                        //if the pixel's fall timer is less than or equal to 0, do this
                        if (pixelspace[x][y].fall_timer_current <= 0) {

                            //reset the fall timer to the pixel's default
                            pixelspace[x][y].fall_timer_current = pixelspace[x][y].fall_timer_default;

                            //add the pixel's position to the fall queue
                            pixelfallqueue[pfq_size] = sf::Vector2i(x, y);

                            //increment the fall queue's size tracker because c++ dosen't have dynamic arrays
                            pfq_size++;
                        }
                    }
                    //set the rectangle's position and color to the pixel's
                    pixelrect.setPosition(x * pixelsize, windowsizeY - ((y + 1) * pixelsize));
                    pixelrect.setFillColor(pixelspace[x][y].color);

                    //queue the rectangle to be drawn
                    window.draw(pixelrect);
                }
            }
        }

        //mouse object
        sf::Mouse mouse;

        //mouse position relative to window
        sf::Vector2i mousepos = mouse.getPosition(window);

        //if mouse is within bounds of window, then...
        if (!(mousepos.x > windowsizeX || mousepos.x < 0 || mousepos.y > windowsizeY || mousepos.y < 0) && window.hasFocus()) {
            //get the mouse position, round it to the position of the pixel tile it's currently on
            sf::Vector2i mousepos_tilemapped = sf::Vector2i(floorf(mousepos.x / pixelsize) * pixelsize, floorf(mousepos.y / pixelsize) * pixelsize);
            //set the pixel preview's planned color to the selected pixel's color
            mouseprevcolor = selectedPixel.color;
            //make the color half transparent
            mouseprevcolor.a = 127;
            //set the pixel preview's fill color fo the planned color
            mouserect.setFillColor(mouseprevcolor);
            //set the preview's position to the grid-adjusted position value
            mouserect.setPosition((sf::Vector2f)mousepos_tilemapped);
            //queue it to be drawn to the window
            window.draw(mouserect);
            //if the left mouse button is pressed, then set the pixel hovered over to the selected pixel
            if (mouse.isButtonPressed(sf::Mouse::Left)) {
                pixelspace[mousepos_tilemapped.x / pixelsize][((windowsizeY - mousepos_tilemapped.y) / pixelsize) - 1] = selectedPixel;
            };
            //if the RIGHT mouse button is pressed, set it to default blank pixel
            if (mouse.isButtonPressed(sf::Mouse::Right)) {
                pixelspace[mousepos_tilemapped.x / pixelsize][((windowsizeY - mousepos_tilemapped.y) / pixelsize) - 1] = Pixel();
            };
            //if the mouse wheel's scroll position was altered, then...
            if (mouseWheelChanged) {
                //add the change amount to the total position
                mouseWheelPos += mouseWheel.delta;
                //if the scroll amount modulo the wheel sensitivity is 0, then...
                if (mouseWheelPos % mouseWheelSensitivity == 0) {
                    /*if the mouse wheel was scrolled up, increment the selected pixel index and
                     *check if the selected pixel index goes over the selectable pixels table.
                     *if it does, loop over to 0.
                     */
                    if (mouseWheel.delta > 0) {
                        selectedPixelIndex++;
                        if (selectedPixelIndex >= pixelEntrySize) {
                            selectedPixelIndex = 0;
                        }
                    }
                    /*if the mouse wheel was scrolled down, decrement the selected pixel index and
                     *check if the selected pixel index goes under zero.
                     *if it does, loop over to the selectable pixels table's max.
                     */
                    if (mouseWheel.delta < 0) {
                        selectedPixelIndex--;
                        if (selectedPixelIndex < 0) {
                            selectedPixelIndex = pixelEntrySize - 1;
                        }
                    }
                    //update the selected pixel and the selected element text
                    selectedPixel = pixelentries[selectedPixelIndex];
                    selectedPixelText.setString("selected: " + pixelentries[selectedPixelIndex].name);
                }
                //the mouse wheel change was dealt with, so this can be set to false now
                mouseWheelChanged = false;
            }
        }

        window.draw(selectedPixelText);
        //and finally, draw all queued objects to the window
        window.display();
    }

    return 0;
}
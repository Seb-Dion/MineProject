#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <chrono>
#include <random>
#include <ctime>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <string>
using namespace std;
using namespace sf;

void setText(Text &text, float x, float y){
    FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    text.setPosition(Vector2f(x, y));
}

struct Tile {
    int row;
    int column;
    Sprite spr;
    bool mineOrNot;
    vector<Tile*> adjacent;
    bool revealed;
    bool flagged;
    int adjacentMines;
    Tile(){
        row = 0;
        column = 0;
        mineOrNot = false;
        revealed = false;
        flagged = false;
        adjacentMines = 0;
    }
    Tile(int row, int column){
        this->row = row;
        this->column = column;
        mineOrNot = false;
        revealed = false;
        adjacentMines = 0;
    }

};

void updateBoard(vector<vector<Tile>> &tileRows, int numMines, map<string, Texture*> &textures){
    mt19937 rng(time(nullptr));
    uniform_int_distribution<int> dist(0,99);
    for(int i = 0; i < tileRows.size(); i++){
        for(int j = 0; j < tileRows[0].size(); j++){
            (tileRows[i])[j].mineOrNot = false;
            (tileRows[i])[j].revealed = false;
            (tileRows[i])[j].flagged = false;
            (tileRows[i])[j].adjacentMines = 0;
            (tileRows[i])[j].adjacent.clear();
            (tileRows[i])[j].spr.setTexture(*textures["tile_hidden.png"]);
        }
    }
    int temp = numMines;
    while(temp > 0) {
        for (int i = 0; i < tileRows.size(); i++) {
            for (int j = 0; j < (tileRows[0]).size(); j++) {
                int random = dist(rng);
                if (temp > 0 && !(tileRows[i])[j].mineOrNot && random == 50) {
                    (tileRows[i])[j].mineOrNot = true;
                    temp -= 1;
                }
            }
        }
    }
    for(int i = 0; i < tileRows.size(); i++){
        for(int j = 0; j < tileRows[0].size(); j++){
            for(int k = -1; k <= 1; k++){
                for(int l = -1; l <= 1; l++){
                    int x = i + k;
                    int y = j + l;
                    if(x >= 0 && x < tileRows.size() && y >= 0 && y < tileRows[0].size()){
                        tileRows[i][j].adjacent.push_back(&tileRows[x][y]);
                    }
                }
            }
        }
    }
    for(int i = 0; i < tileRows.size(); i++){
        for(int j = 0; j < tileRows[0].size(); j++){
            for(int k = 0; k < (tileRows[i])[j].adjacent.size(); k++){
                if((tileRows[i])[j].adjacent[k]->mineOrNot){
                    (tileRows[i])[j].adjacentMines++;
                }
            }
        }
    }
}
void updateTilesRecursive(Tile &tiles, int &revealed){
    tiles.revealed = true;
    revealed += 1;
    for(auto& i : tiles.adjacent){
        if(!(i->mineOrNot) && !(i->flagged) && !(i->revealed)){
            if(i->adjacentMines == 0){
                updateTilesRecursive(*i, revealed);
            }
        }
        if(i->adjacentMines != 0 && !(i->mineOrNot) && !(i->flagged) && !(i->revealed)){
            revealed += 1;
            i->revealed = true;
        }
    }
}
void updateLeaderboard(string &input, string player, bool gameStatus, int time){
    int players = 5;
    vector<string> temp(players);
    ifstream file("files/leaderboard.txt");
    getline(file, temp[0]);
    getline(file, temp[1]);
    getline(file, temp[2]);
    getline(file, temp[3]);
    getline(file, temp[4]);

    if(gameStatus == true){
        for(int i = 0; i < players; i++){
            if((stoi(temp[i].substr(0,2)) * 60) + (stoi(temp[i].substr(3,2))) > time) {
                string tempPlayer;
                if((time / 60) < 10){
                    tempPlayer = "0" + to_string(time/60) + ":";
                } else {
                    tempPlayer = to_string(time/60) + ":";
                }
                if((time % 60) < 10){
                    tempPlayer += "0" + to_string(time % 60);
                } else {
                    tempPlayer += to_string(time % 60);
                }
                tempPlayer += ", " + player + "*";
                temp.insert(temp.begin() + i, tempPlayer);
                temp.pop_back();
                break;
            }
        }
        ofstream outfile("files/leaderboard.txt");
        for(string playa : temp){
            outfile << playa << endl;
        }
    }
    input = "";
    for (int i = 0; i < 5; i++) {
        input += to_string(i + 1) + ".\t" + temp[i].substr(0, 5) + "\t" + temp[i].substr(6, temp[i].size() - 5);
        if (i < 4 && i < temp.size() - 1)
            input += "\n\n";
    }
}



int main() {
    ifstream file("files/config.cfg");
    string singleLine;
    int colCount;
    int rowCount;
    int numMines;
    int revealed;
    getline(file, singleLine);
    colCount = stoi(singleLine);
    getline(file, singleLine);
    rowCount = stoi(singleLine);
    getline(file, singleLine);
    numMines = stoi(singleLine);
    int height = (rowCount * 32) + 100;
    int width = (colCount * 32);
    int flags = numMines;
    string bestPlayers;


    map<string, Texture*> textures;
    Texture debug;
    debug.loadFromFile("files/images/debug.png");
    textures.emplace("debug.png", &debug);
    Texture digits;
    digits.loadFromFile("files/images/digits.png");
    textures.emplace("digits.png", &digits);
    Texture face_happy;
    face_happy.loadFromFile("files/images/face_happy.png");
    textures.emplace("face_happy.png", &face_happy);
    Texture face_lose;
    face_lose.loadFromFile("files/images/face_lose.png");
    textures.emplace("face_lose.png", &face_lose);
    Texture face_win;
    face_win.loadFromFile("files/images/face_win.png");
    textures.emplace("face_win.png", &face_win);
    Texture flag;
    flag.loadFromFile("files/images/flag.png");
    textures.emplace("flag.png", &flag);
    Texture pauseButton;
    pauseButton.loadFromFile("files/images/pause.png");
    textures.emplace("pause.png", &pauseButton);
    Texture leaderboardButton;
    leaderboardButton.loadFromFile("files/images/leaderboard.png");
    textures.emplace("leaderboard.png", &leaderboardButton);
    Texture mineText;
    mineText.loadFromFile("files/images/mine.png");
    textures.emplace("mine.png", &mineText);
    Texture tile_revealed;
    tile_revealed.loadFromFile("files/images/tile_revealed.png");
    textures.emplace("tile_revealed.png", &tile_revealed);
    Texture playButton;
    playButton.loadFromFile("files/images/play.png");
    textures.emplace("play.png", &playButton);
    Texture number_1;
    number_1.loadFromFile("files/images/number_1.png");
    textures.emplace("number_1", &number_1);
    Texture number_2;
    number_2.loadFromFile("files/images/number_2.png");
    textures.emplace("number_2", &number_2);
    Texture number_3;
    number_3.loadFromFile("files/images/number_3.png");
    textures.emplace("number_3", &number_3);
    Texture number_4;
    number_4.loadFromFile("files/images/number_4.png");
    textures.emplace("number_4", &number_4);
    Texture number_5;
    number_5.loadFromFile("files/images/number_5.png");
    textures.emplace("number_5", &number_5);
    Texture number_6;
    number_6.loadFromFile("files/images/number_6.png");
    textures.emplace("number_6", &number_6);
    Texture number_7;
    number_7.loadFromFile("files/images/number_7.png");
    textures.emplace("number_7", &number_7);
    Texture number_8;
    number_8.loadFromFile("files/images/number_8.png");
    textures.emplace("number_8", &number_8);


    bool check = false;
    RenderWindow welcome(VideoMode(width, height), "Minesweeper", Style::Close);
    Font font;
    font.loadFromFile("files/font.ttf");

    Text title;
    title.setFont(font);
    title.setString("WELCOME TO MINESWEEPER!");
    title.setCharacterSize(24);
    title.setFillColor(Color::White);
    title.setStyle(Text::Bold);
    title.setStyle(Text::Underlined);
    setText(title, width / 2, (height / 2) - 150);

    Text subTitle;
    subTitle.setFont(font);
    subTitle.setString("Enter your name:");
    subTitle.setFillColor(Color::White);
    subTitle.setCharacterSize(20);
    subTitle.setStyle(Text::Bold);
    setText(subTitle, width / 2, (height / 2) - 75);

    string name = "|";
    Text nameInput(name, font, 18);
    char letter;
    nameInput.setFillColor(Color::Yellow);
    setText(nameInput, width / 2, (height / 2) - 45);

    while (welcome.isOpen()) {
        Event event;
        while (welcome.pollEvent(event)) {
            if (event.type == Event::Closed) {
                welcome.close();
            }
            if (event.type == Event::TextEntered) {
                char letter = static_cast<char>(event.text.unicode);
                if (isalpha(letter) && name.size() <= 10) {
                    if (name == "|") {
                        letter = toupper(letter);
                    } else {
                        letter = tolower(letter);
                    }
                    if (name.size() == 10 && name[9] == '|') {
                        name[9] = letter;
                    } else if (name.size() < 10) {
                        name[name.size() - 1] = letter;
                        name += "|";
                    }
                } else if (event.text.unicode == '\b' && name[name.size() - 1] != '|') {
                    name.pop_back();
                    name += "|";
                } else if (event.text.unicode == '\b' && name[name.size() - 1] == '|' && name.size() > 1) {
                    name.pop_back();
                    name.pop_back();
                    name += "|";
                }

                nameInput.setString(name);
                setText(nameInput, width / 2, (height / 2) - 45);
            }
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Return && name.size() > 1) {
                    if (name[name.size() - 1] == '|' && name.size() != 1) {
                        name = name.substr(0, name.size() - 1);
                    }
                    welcome.close();
                    check = true;
                }
            }
        }

        welcome.clear(Color::Blue);
        welcome.draw(title);
        welcome.draw(subTitle);
        welcome.draw(nameInput);
        welcome.display();
    }
    if (check) {
        bool gameStatus = false;
        Texture tile_hidden;
        tile_hidden.loadFromFile("files/images/tile_hidden.png");
        Sprite tile;
        tile.setTexture(tile_hidden);
        textures.emplace("tile_hidden.png", &tile_hidden);
        Sprite face;
        face.setTexture(face_happy);
        face.setPosition(((colCount / 2.0) * 32) - 32, (32 * (rowCount + 0.5)));
        Sprite debugger;
        debugger.setTexture(debug);
        debugger.setPosition((colCount * 32) - 304, (rowCount + 0.5) * 32);
        bool checkDebug = false;
        Sprite pause;
        pause.setTexture(pauseButton);
        pause.setPosition((colCount * 32) - 240, 32 * (rowCount + 0.5));
        bool checkPause = false;
        bool boardStatus = false;
        Sprite leaderBoarder;
        leaderBoarder.setTexture(leaderboardButton);
        leaderBoarder.setPosition((colCount * 32) - 176, 32 * (rowCount + 0.5));


        RenderWindow game(VideoMode(width, height), "Minesweeper", Style::Close);
        vector<vector<Tile>> tileRows(rowCount);

        for(int i = 0; i < rowCount; i++){
            vector<Tile> currentRow(colCount);
            for(int j = 0; j < colCount; j++){
                Tile curr(i,j);
                curr.spr.setPosition(j * 32, i * 32);
                curr.spr.setTexture(*textures["tile_hidden.png"]);
                currentRow[j] = curr;
            }
            tileRows[i] = currentRow;
        }

        updateBoard(tileRows, numMines, textures);
        auto begin = chrono::high_resolution_clock::now();
        auto zero = begin;
        float time = 0.0;
        float elapsed = 0.0;

        while (game.isOpen()) {
            Event event;
            while (game.pollEvent(event)) {
                if (event.type == Event::Closed) {
                    game.close();
                }
                if (event.type == Event::MouseButtonPressed) {
                    if (event.mouseButton.button == Mouse::Left) {
                        FloatRect bounds = leaderBoarder.getGlobalBounds();
                        Vector2f mousePosition = game.mapPixelToCoords(Mouse::getPosition(game));
                        if (bounds.contains(mousePosition)) {
                            boardStatus = true;
                            checkPause = true;
                        }
                        if (!gameStatus) {
                            FloatRect pauseBounds = pause.getGlobalBounds();
                            if (pauseBounds.contains(mousePosition)) {
                                checkPause = !checkPause;
                                if (!checkPause) {
                                    pause.setTexture(*textures["pause.png"]);
                                } else {
                                    pause.setTexture(*textures["play.png"]);
                                }
                            }
                        }
                        if ((!checkPause || gameStatus) && !boardStatus) {
                            FloatRect faceBounds = face.getGlobalBounds();
                            if (faceBounds.contains(mousePosition)) {
                                face.setTexture(*textures["face_happy.png"]);
                                revealed = 0;
                                begin = chrono::high_resolution_clock::now();
                                time = 0;
                                updateBoard(tileRows, numMines, textures);
                                checkPause = false;
                                gameStatus = false;
                                checkDebug = false;
                                flags = numMines;
                            }
                        }
                        if (!checkPause && !gameStatus) {
                            FloatRect debugBounds = debugger.getGlobalBounds();
                            if (debugBounds.contains(mousePosition)) {
                                checkDebug = !checkDebug;
                            }
                            for (int i = 0; i < rowCount; i++) {
                                for (int j = 0; j < colCount; j++) {
                                    FloatRect tileBounds = (tileRows[i])[j].spr.getGlobalBounds();
                                    if (tileBounds.contains(mousePosition) && !(tileRows[i])[j].revealed &&
                                        !(tileRows[i])[j].flagged) {
                                        if ((tileRows[i])[j].adjacentMines == 0) {
                                            updateTilesRecursive((tileRows[i])[j], revealed);
                                        } else if ((tileRows[i])[j].mineOrNot == true) {
                                            gameStatus = true;
                                            boardStatus = true;
                                            (tileRows[i])[j].spr.setTexture(*textures["tile_revealed.png"]);
                                            face.setTexture(*textures["face_lose.png"]);
                                        } else if ((tileRows[i])[j].adjacentMines > 0) {
                                            revealed++;
                                            tileRows[i][j].revealed = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if(event.mouseButton.button == Mouse::Right) {
                        Vector2f mousePosition = game.mapPixelToCoords(Mouse::getPosition(game));
                        if (!checkPause && !gameStatus)
                            for (int i = 0; i < rowCount; i++) {
                                for (int j = 0; j < colCount; j++) {
                                    FloatRect tileButton = (tileRows[i])[j].spr.getGlobalBounds();
                                    if (tileButton.contains(mousePosition)) {
                                        if (!(tileRows[i])[j].revealed) {
                                            if ((tileRows[i])[j].flagged)
                                                flags++;
                                            else
                                                flags--;
                                            (tileRows[i])[j].flagged = !(tileRows[i])[j].flagged;
                                        }
                                    }
                                }
                            }
                    }
                }
            }
            game.clear(Color::White);
            if(revealed == rowCount*colCount - numMines){
                face.setTexture(*textures["face_win.png"]);
                boardStatus = true;
                gameStatus = true;
                updateLeaderboard(bestPlayers,name, gameStatus, (int)elapsed);
            }

            if(gameStatus && revealed == rowCount*colCount - numMines){
                for(int i = 0; i < rowCount; i++){
                    for(int j = 0; j < colCount; j++){
                        if(tileRows[i][j].mineOrNot){
                            tileRows[i][j].flagged = true;
                        }
                    }
                }
            } else if(gameStatus && revealed < rowCount * colCount - numMines){
                for(int i = 0; i < rowCount; i++){
                    for(int j = 0; j < colCount; j++){
                        if(tileRows[i][j].mineOrNot){
                            tileRows[i][j].revealed = true;
                        }
                    }
                }
            }
            for (int i = 0; i < rowCount; i++) {
                for (int j = 0; j < colCount; j++) {
                    game.draw((tileRows[i])[j].spr);
                    if((tileRows[i])[j].mineOrNot && (tileRows[i])[j].revealed){
                        Sprite newTile;
                        newTile.setTexture(*textures["mine.png"]);
                        newTile.setPosition(j*32, i*32);
                        game.draw(newTile);
                    }
                }
            }
            for (int i = 0; i < rowCount; i++) {
                for (int j = 0; j < colCount; j++) {
                    game.draw((tileRows[i])[j].spr);
                    if ((tileRows[i])[j].revealed)
                        (tileRows[i])[j].spr.setTexture(*textures["tile_revealed.png"]);
                    if ((tileRows[i])[j].mineOrNot && (tileRows[i])[j].revealed) {
                        Sprite tempMine;
                        tempMine.setPosition((float) j * 32, (float) i * 32);
                        tempMine.setTexture(*textures["mine.png"]);
                        game.draw(tempMine);
                    } else if ((tileRows[i])[j].flagged) {
                        Sprite tempFlag;
                        tempFlag.setPosition((float) j * 32, (float) i * 32);
                        tempFlag.setTexture(*textures["flag.png"]);
                        game.draw(tempFlag);
                    } else if ((tileRows[i])[j].adjacentMines > 0 && (tileRows[i])[j].revealed) {
                        Sprite temp;
                        temp.setPosition(j * 32, i * 32);
                        temp.setTexture(*textures["number_" + to_string((tileRows[i])[j].adjacentMines)]);
                        game.draw(temp);
                    }
                }
            }

            if(checkPause){
                for (int i = 0; i < rowCount; i++) {
                    for (int j = 0; j < colCount; j++) {
                        Sprite newTile;
                        newTile.setTexture(*textures["tile_revealed.png"]);
                        newTile.setPosition(j*32, i*32);
                        game.draw(newTile);
                    }
                }
            } else if(checkDebug){
                for (int i = 0; i < rowCount; i++) {
                    for (int j = 0; j < colCount; j++) {
                        if((tileRows[i])[j].mineOrNot && !(tileRows[i])[j].revealed){
                            Sprite newTile;
                            newTile.setTexture(*textures["mine.png"]);
                            newTile.setPosition(j*32, i*32);
                            game.draw(newTile);
                        }
                    }
                }
            }

            game.draw(face);
            game.draw(debugger);
            game.draw(pause);
            game.draw(leaderBoarder);

            Sprite tempNumber;
            int yVal = 32*(rowCount+0.5) + 16;
            int xVal = colCount * 32;
            if(flags < 0){
                tempNumber.setPosition(12, yVal);
                tempNumber.setTexture(*textures["digits.png"]);
                tempNumber.setTextureRect(IntRect(210,0,21,32));
                game.draw(tempNumber);
            }
            tempNumber.setPosition(33,yVal);
            tempNumber.setTexture(*textures["digits.png"]);
            tempNumber.setTextureRect(IntRect(abs(flags/100) * 21, 0, 21, 32));
            game.draw(tempNumber);
            tempNumber.setPosition(54,yVal);
            tempNumber.setTexture(*textures["digits.png"]);
            tempNumber.setTextureRect(IntRect(abs((flags % 100) / 10) * 21, 0, 21, 32));
            game.draw(tempNumber);
            tempNumber.setPosition(75,yVal);
            tempNumber.setTexture(*textures["digits.png"]);
            tempNumber.setTextureRect(IntRect(abs(flags%10) * 21, 0, 21, 32));
            game.draw(tempNumber);

            auto tempTime = chrono::high_resolution_clock::now();
            if(!checkPause){
                elapsed = chrono::duration_cast<chrono::seconds>(tempTime - begin).count() + time;
            }
            if(checkPause || gameStatus){
                begin = tempTime;
                time = elapsed;
            }
            tempNumber.setPosition(xVal-97,yVal);
            tempNumber.setTexture(*textures["digits.png"]);
            tempNumber.setTextureRect(IntRect((int)elapsed/60/10*21,0,21,32));
            game.draw(tempNumber);
            tempNumber.setPosition(xVal-76,yVal);
            tempNumber.setTexture(*textures["digits.png"]);
            tempNumber.setTextureRect(IntRect((int)elapsed/60%10*21,0,21,32));
            game.draw(tempNumber);
            tempNumber.setPosition(xVal-54,yVal);
            tempNumber.setTexture(*textures["digits.png"]);
            tempNumber.setTextureRect(IntRect((int)elapsed%60/10*21,0,21,32));
            game.draw(tempNumber);
            tempNumber.setPosition(xVal-33,yVal);
            tempNumber.setTexture(*textures["digits.png"]);
            tempNumber.setTextureRect(IntRect((int)elapsed%60%10*21,0,21,32));
            game.draw(tempNumber);
            game.display();
            if(boardStatus == true){
                Text leaderTitle("LEADERBOARD", font, 20);
                Text currLeaders(bestPlayers, font, 18);
                currLeaders.setFillColor(Color::White);
                currLeaders.setStyle(Text::Bold);
                updateLeaderboard(bestPlayers,name,gameStatus,6000);
                currLeaders.setString(bestPlayers);
                RenderWindow leaderBoard(VideoMode(colCount*16, rowCount*16+50), "Leaderboard", Style::Close);
                while(leaderBoard.isOpen()) {
                    Event event;
                    while (leaderBoard.pollEvent(event)) {
                        if (event.type == Event::Closed) {
                            leaderBoard.close();
                            boardStatus = false;
                            revealed++;
                            checkPause = false;
                            ifstream newFile("files/leaderboard.txt");
                            char temp;
                            string tempLeader;
                            while (newFile.get(temp)) {
                                if (temp != '*')
                                    tempLeader += temp;
                            }
                            ofstream outputFile("files/leaderboard.txt");
                            outputFile << tempLeader;
                        }
                    }
                    tempTime = chrono::high_resolution_clock::now();
                    begin = tempTime;
                    time = elapsed;
                    leaderBoard.clear(Color::Blue);
                    setText(leaderTitle, leaderBoard.getSize().x/2, leaderBoard.getSize().y/2 - 120);
                    setText(currLeaders, leaderBoard.getSize().x/2, leaderBoard.getSize().y/2 + 20);
                    leaderBoard.draw(leaderTitle);
                    leaderBoard.draw(currLeaders);
                    leaderBoard.display();
                }
            }
        }
    }
    return 0;
}
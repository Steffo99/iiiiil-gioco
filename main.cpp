#include <iostream>
#define X_MAX 80
#define Y_MAX 23

using namespace std;

//Aggiorna la console con la situazione corrente del gioco.
void draw(char map[X_MAX][Y_MAX])
{
    for(int y=0; y<Y_MAX; y++)
    {
        for(int x=0; x<X_MAX; x++)
        {
            cout << map[x][y];
        }
    }
}

//Funzioni per la generazione della mappa
//Inizializza la mappa con spazi vuoti
void init(char map[X_MAX][Y_MAX])
{
    for(int y=0; y<Y_MAX; y++)
    {
        for(int x=0; x<X_MAX; x++)
        {
            map[x][y] = ' ';
        }
    }
}

//Crea una stanza quadrata
void room(char map[X_MAX][Y_MAX], int start_x, int start_y, int end_x, int end_y)
{
    for(int y=start_y; y<=end_y; y++)
    {
        for(int x=start_x; x<=end_x; x++)
        {
            map[x][y] = '.';
        }
    }
}

//Crea un corridoio che connetta due punti
void corridor(char map[X_MAX][Y_MAX], int start_x, int start_y, int end_x, int end_y, bool verticale)
{
    if(verticale)
    {
        if(start_y > end_y)
        {
            for(int y=end_y; y<=start_y; y++)
            {
                map[start_x][y] = '.';
            }
        }
        else
        {
            for(int y=start_y; y<=end_y; y++)
            {
                map[start_x][y] = '.';
            }
        }
        if(start_x > end_x)
        {
            for(int x=end_x; x<=start_x; x++)
            {
                map[x][end_y] = '.';
            }
        }
        else
        {
            for(int x=start_x; x<=end_x; x++)
            {
                map[x][end_y] = '.';
            }
        }
    }
    else
    {
        if(start_x > end_x)
        {
            for(int x=end_x; x<=start_x; x++)
            {
                map[x][start_y] = '.';
            }
        }
        else
        {
            for(int x=start_x; x<=end_x; x++)
            {
                map[x][start_y] = '.';
            }
        }
        if(start_y > end_y)
        {
            for(int y=end_y; y<=start_y; y++)
            {
                map[end_x][y] = '.';
            }
        }
        else
        {
            for(int y=start_y; y<=end_y; y++)
            {
                map[end_x][y] = '.';
            }
        }
    }
}

int main()
{
    char map[X_MAX][Y_MAX]; //Mappa del gioco
    init(map);
    corridor(map, 1, 1, 3, 3, true);
    corridor(map, 5, 5, 7, 7, false);
    draw(map);
    return 0;
}

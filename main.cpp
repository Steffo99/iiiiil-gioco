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

int main()
{
    char map[X_MAX][Y_MAX]; //Mappa del gioco
    init(map);
    draw(map);
    return 0;
}

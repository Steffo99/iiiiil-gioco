#include <iostream>
#include <random>
#include <conio.h>
#include <stdlib.h>

#define X_MAX 80
#define Y_MAX 23
#define ROOMS 8
#define ROOM_SIZE 7
#define ENEMIES_IN_LEVEL 5
#define HP_MAX 50
#define STARTING_ATK 5
#define MAX_POTIONS_PER_FLOOR 5

using namespace std;

const unsigned char WALL = 0xB2;
const unsigned char EMPTY = ' ';
const unsigned char PLAYER = 0x02;
const unsigned char ENEMY = 'X';
const unsigned char EXIT = '>';
const unsigned char DOUBLELINE = 0xCD;
const unsigned char ITEM_SMALL_POTION = 'p';
const unsigned char ITEM_MEDIUM_POTION = 'n';
const unsigned char ITEM_BIG_POTION = 'm';

//Mappa del gioco
unsigned char map[X_MAX][Y_MAX];

//Numero del piano raggiunto dal giocatore
int depth = 1;

//Devo mettere due volte draw perchè ha bisogno della classe Player, a cui però serve la funzione draw. Idem per l'inventario...
void draw();
void inventory();

//Classe entità generica, sia nemico sia giocatore
class Entity
{
    public:
        int x;
        int y;
        int hp = HP_MAX;
        int hp_max = HP_MAX;
        int move();
        //Cura di x l'entità
        void heal(int x)
        {
            if(hp + x > hp_max)
            {
                hp = hp_max;
            }
            else
            {
                hp += x;
            }
        }
        //Danneggia di x l'entità
        void damage(int x)
        {
            if(hp - x < 0)
            {
                //entità zombi
                hp = 0;
            }
            else
            {
                hp -= x;
            }
        }
};

//Classe del giocatore
class Player : public Entity
{
    private:
        int equipment_atk = 0; //Attacco ottenuto dall'equipaggiamento
    public:
        int base_atk = STARTING_ATK; //Attacco di base
        int atk()
        {
            int result = equipment_atk + base_atk;
            return result;
        }
        int pozioni_vita_piccole = 3;
        int pozioni_vita_medie = 2;
        int pozioni_vita_grandi = 1;
        int move()
        {
            bool waiting = true;
            unsigned char* starting = &map[x][y]; //Casella attuale
            unsigned char* target; //Bersaglio del movimento
            //Rileva i tasti freccia
            while(waiting)
            {
                unsigned char input = getch();
                short int dir = 0; //Direzione del movimento, per modificare x e y.
                if(input == 224)
                {
                    switch(getch())
                    {
                        case 72: //Freccia su, 1
                            target = &map[x][y-1];
                            dir = 1;
                            break;
                        case 80: //Freccia giù, 2
                            target = &map[x][y+1];
                            dir = 2;
                            break;
                        case 75: //Freccia sinistra, 3
                            target = &map[x-1][y];
                            dir = 3;
                            break;
                        case 77: //Freccia destra, 4
                            target = &map[x+1][y];
                            dir = 4;
                            break;
                        //Aggiungere gestione del caso che non sia una delle quattro frecce
                    }
                    //Muoviti e agisci!
                    if(*target == EMPTY)
                    {
                        *starting = EMPTY;
                        *target = PLAYER;
                        waiting = false;
                    }
                    else if(*target == ITEM_SMALL_POTION)
                    {
                        *starting = EMPTY;
                        *target = PLAYER;
                        pozioni_vita_piccole++;
                        waiting = false;
                    }
                    else if(*target == ITEM_MEDIUM_POTION)
                    {
                        *starting = EMPTY;
                        *target = PLAYER;
                        pozioni_vita_medie++;
                        waiting = false;
                    }
                    else if(*target == ITEM_BIG_POTION)
                    {
                        *starting = EMPTY;
                        *target = PLAYER;
                        pozioni_vita_grandi++;
                        waiting = false;
                    }
                    else if(*target == EXIT)
                    {
                        return 1;
                    }
                }
                else if(input == 's') //S
                {
                    //Salta un turno
                    waiting = false;
                }
                else if(input == 'i') //I
                {
                    //Apri l'inventario
                    inventory();
                    //Torna alla mappa
                    draw();
                }
                //Se ti sei mosso, controlla in che direzione e aggiorna correttamente la x e la y.
                //Non mi veniva in mente un modo per farlo meglio.
                if(!waiting)
                {
                    if(dir == 1)
                    {
                        y--;
                    }
                    else if(dir == 2)
                    {
                        y++;
                    }
                    else if(dir == 3)
                    {
                        x--;
                    }
                    else if(dir == 4)
                    {
                        x++;
                    }
                }
            }
            return 0;
        }
} player;

//Classe dei nemici
class Enemy : public Entity
{
    public:
        int move()
        {
            //Se intorno c'è il giocatore
            if(map[x-1][y] == PLAYER || map[x+1][y] == PLAYER || map[x][y-1] == PLAYER || map[x][y+1] == PLAYER)
            {
                //Forse sarebbe meglio fare una funzione per togliere vita che controlla anche se va a 0...
                player.damage(rand() % 5 + 1);
            }
            else
            {
                //Se il giocatore è vicino, muoviti verso di lui
                if(map[x-2][y] == PLAYER && map[x-1][y] == EMPTY) //Due a sinistra
                {
                    map[x][y] = EMPTY;
                    map[x-1][y] = ENEMY;
                    x--;
                }
                else if(map[x+2][y] == PLAYER && map[x+1][y] == EMPTY) //Due a destra
                {
                    map[x][y] = EMPTY;
                    map[x+1][y] = ENEMY;
                    x++;
                }
                else if(map[x][y-2] == PLAYER && map[x][y-1] == EMPTY) //Due in su
                {
                    map[x][y] = EMPTY;
                    map[x][y-1] = ENEMY;
                    y--;
                }
                else if(map[x][y+2] == PLAYER && map[x][y+1] == EMPTY) //Due in giù
                {
                    map[x][y] = EMPTY;
                    map[x][y+1] = ENEMY;
                    y++;
                }
                else
                {
                    //Muoviti in una direzione casuale
                    bool moving = true;
                    while(moving)
                    {
                        int direction = rand() % 4;
                        switch(direction)
                        {
                            case 0: //Sinistra
                                if(map[x-1][y] == EMPTY)
                                {
                                    map[x][y] = EMPTY;
                                    map[x-1][y] = ENEMY;
                                    x--;
                                    moving = false;
                                }
                                break;
                            case 1: //Destra
                                if(map[x+1][y] == EMPTY)
                                {
                                    map[x][y] = EMPTY;
                                    map[x+1][y] = ENEMY;
                                    x++;
                                    moving = false;
                                }
                                break;
                            case 2: //Su
                                if(map[x][y-1] == EMPTY)
                                {
                                    map[x][y] = EMPTY;
                                    map[x][y-1] = ENEMY;
                                    y--;
                                    moving = false;
                                }
                                break;
                            case 3: //Giù
                                if(map[x][y+1] == EMPTY)
                                {
                                    map[x][y] = EMPTY;
                                    map[x][y+1] = ENEMY;
                                    y++;
                                    moving = false;
                                }
                                break;
                        }
                    }
                }
            }
        }
};

//Aggiorna la console con la situazione corrente del gioco.
void draw()
{
    //Svuota lo schermo della console. Sono sicuro che ci sia un modo molto migliore per farlo, ma non mi viene in mente...
    system("cls");
    for(int y=0; y<Y_MAX; y++)
    {
        for(int x=0; x<X_MAX; x++)
        {
            printf("%c", map[x][y]);
        }
    }
    printf("Piano: %d | Vita: %d/%d | x:%d y:%d | atk: %d\n", depth, player.hp, HP_MAX, player.x, player.y, player.atk());
}

//Visualizza l'inventario
void inventory()
{
    system("cls");
    printf("Piano: %d | Vita: %d/%d | x:%d y:%d | atk: %d\n", depth, player.hp, HP_MAX, player.x, player.y, player.atk());
    for(int i = 0; i < X_MAX; i++)
    {
        printf("%c", (char) DOUBLELINE);
    }
    //Anche qui, credo si possa migliorare qualcosa...
    if(player.pozioni_vita_piccole > 0)
    {
        printf("%dx Pozione di Vita (p)iccola\tRipristina 10 Vita\n", player.pozioni_vita_piccole);
    }
    else
    {
        printf("\n");
    }
    if(player.pozioni_vita_medie > 0)
    {
        printf("%dx Pozione di Vita (n)ormale\tRipristina 20 Vita\n", player.pozioni_vita_medie);
    }
    else
    {
        printf("\n");
    }
    if(player.pozioni_vita_grandi > 0)
    {
        printf("%dx Pozione di Vita (m)aggiore\tRipristina 50 Vita\n", player.pozioni_vita_grandi);
    }
    else
    {
        printf("\n");
    }
    for(int i = 0; i < X_MAX; i++)
    {
        printf("%c", (char) DOUBLELINE);
    }
    //Selezione dell'oggetto da usare.
    printf("Scrivi la lettera corrispondente all'oggetto che vuoi usare.\nEsci con Esc.\n");
    while(true)
    {
        //Effetto degli oggetti
        unsigned char selezione = getch();
        if(selezione == 112) //p
        {
            if(player.pozioni_vita_piccole > 0)
            {
                player.pozioni_vita_piccole--;
                player.heal(10);
                break;
            }
        }
        else if(selezione == 110) //n
        {
            if(player.pozioni_vita_medie > 0)
            {
                player.pozioni_vita_medie--;
                player.heal(20);
                break;
            }
        }
        else if(selezione == 109) //m
        {
            if(player.pozioni_vita_grandi > 0)
            {
                player.pozioni_vita_grandi--;
                player.heal(50);
                break;
            }
        }
        else if(selezione == 27) //esc
        {
            break;
        }
    }
}

//Funzioni per la generazione della mappa
//Inizializza la mappa con spazi vuoti
void init()
{
    for(int y=0; y<Y_MAX; y++)
    {
        for(int x=0; x<X_MAX; x++)
        {
            map[x][y] = WALL;
        }
    }
}

//Crea una stanza quadrata
void room(int start_x, int start_y, int end_x, int end_y)
{
    for(int y=start_y; y<=end_y; y++)
    {
        for(int x=start_x; x<=end_x; x++)
        {
            map[x][y] = EMPTY;
        }
    }
}

//Crea un corridoio che connetta due punti
void corridor(int start_x, int start_y, int end_x, int end_y, bool verticale)
{
    if(verticale)
    {
        if(start_y > end_y)
        {
            for(int y=end_y; y<=start_y; y++)
            {
                map[start_x][y] = EMPTY;
            }
        }
        else
        {
            for(int y=start_y; y<=end_y; y++)
            {
                map[start_x][y] = EMPTY;
            }
        }
        if(start_x > end_x)
        {
            for(int x=end_x; x<=start_x; x++)
            {
                map[x][end_y] = EMPTY;
            }
        }
        else
        {
            for(int x=start_x; x<=end_x; x++)
            {
                map[x][end_y] = EMPTY;
            }
        }
    }
    else
    {
        if(start_x > end_x)
        {
            for(int x=end_x; x<=start_x; x++)
            {
                map[x][start_y] = EMPTY;
            }
        }
        else
        {
            for(int x=start_x; x<=end_x; x++)
            {
                map[x][start_y] = EMPTY;
            }
        }
        if(start_y > end_y)
        {
            for(int y=end_y; y<=start_y; y++)
            {
                map[end_x][y] = EMPTY;
            }
        }
        else
        {
            for(int y=start_y; y<=end_y; y++)
            {
                map[end_x][y] = EMPTY;
            }
        }
    }
}

//Genera il livello
void generate(Enemy* list[ENEMIES_IN_LEVEL])
{
    int corridor_x;
    int corridor_y;
    //Creazione delle stanze
    for(int r=0; r<ROOMS; r++)
    {
        int size_x = rand() % ROOM_SIZE + 1;
        int size_y = rand() % ROOM_SIZE + 1;
        int start_x = rand() % (X_MAX - size_x - 2) + 1;
        int start_y = rand() % (Y_MAX - size_y - 2) + 1;
        room(start_x, start_y, start_x + size_x, start_y + size_y);
        //Se non è la prima stanza, crea un corridoio che connetta quella appena generata con quella precedente
        if(r > 0)
        {
            int link_x = rand() % size_x + 1 + start_x;
            int link_y = rand() % size_y + 1 + start_y;
            corridor(link_x, link_y, corridor_x, corridor_y, rand() % 2);
        }
        corridor_x = rand() % size_x + start_x;
        corridor_y = rand() % size_y + start_y;
        //Posiziona il giocatore se è l'ultima stanza
        if(r == ROOMS - 1)
        {
            map[corridor_x][corridor_y] = PLAYER;
            player.x = corridor_x;
            player.y = corridor_y;
        }
    }
    //Posizionamento nemici
    for(int e=0; e<ENEMIES_IN_LEVEL; e++)
    {
        while(true)
        {
            int x = rand() % (X_MAX - 1) + 1;
            int y = rand() % (Y_MAX - 1) + 1;
            if(map[x][y] == EMPTY)
            {
                map[x][y] = ENEMY;
                Enemy* created = new Enemy();
                created->x = x;
                created->y = y;
                list[e] = created;
                break;
            }
        }
    }
    //Posizionamento uscita
    while(true)
    {
        int x = rand() % (X_MAX - 1) + 1;
        int y = rand() % (Y_MAX - 1) + 1;
        if(map[x][y] == EMPTY)
        {
            map[x][y] = EXIT;
            break;
        }
    }
    //Posizionamento pozioni di vita
    int placed_potions = 0;
    int potions_in_floor = rand() % MAX_POTIONS_PER_FLOOR;
    while(placed_potions < potions_in_floor)
    {
        int x = rand() % (X_MAX - 1) + 1;
        int y = rand() % (Y_MAX - 1) + 1;
        int size = rand() % 100;
        if(map[x][y] == EMPTY)
        {
            if(size < 60)
            {
                map[x][y] = ITEM_SMALL_POTION;
            }
            else if(size > 90)
            {
                map[x][y] = ITEM_BIG_POTION;
            }
            else
            {
                map[x][y] = ITEM_MEDIUM_POTION;
            }
            placed_potions++;
        }
    }
}

//Processa il resto di un turno, dopo il movimento del giocatore.
void tick(Enemy* list[ENEMIES_IN_LEVEL])
{
    for(int e=0; e<ENEMIES_IN_LEVEL; e++)
    {
        list[e]->move();
    }
}

int main()
{
    Enemy* list[ENEMIES_IN_LEVEL]; //Lista di tutti i nemici nel livello
    int seed; //Seed casuale per generare il livello
    printf("Seleziona un seed per la partita: ");
    cin >> seed;
    srand(seed);
    //Ciclo del gioco
    while(true)
    {
        init();
        generate(list);
        draw();
        //Ciclo di un livello
        while(true)
        {
            int trigger = player.move();
            if(trigger == 1) //Uscita toccata
            {
                break;
            }
            tick(list);
            draw();
        }
        depth++;
    }
    return 0;
}

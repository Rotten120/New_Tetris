#include "raylib.h"
#include "math.h"

/*
    REMAINING MECHANICS TO ADD:
    VISUAL GRAPHICS (35%)
    WALL KICKS AND FOOT KICKS (2%)
*/

#define SETFPS 60
#define BOXSIZE 30
#define BOARDWIDTH 10
#define BOARDHEIGHT 21

enum windowStates
{
    WINDOWSTATE_PLAY,
    WINDOWSTATE_PAUSE,
    WINDOWSTATE_GAMEOVER
};

enum pieceShapes
{
    O_PIECE,
    I_PIECE,
    L_PIECE,
    RL_PIECE,
    S_PIECE,
    RS_PIECE,
    T_PIECE
};

struct board
{
    bool IsBlank;
    Vector2 Coords;
    Color color;
};

struct piece
{
    enum pieceShapes shape;
    Vector2 Coords[4];
    Color color;  
};

struct frameTimer
{
    int counter;
    int max;
};

struct gameStats
{
    int Lines;
    int CurrentTimer;
    int Score;
};

static const int screenWidth = 700;
static const int screenHeight = 800;

struct frameTimer slow_Timer, drop_Timer, move_Timer;
struct gameStats Stats;

bool CanPieceRotate;
int PieceRotation;

struct board Board[BOARDWIDTH][BOARDHEIGHT];
struct piece MainPiece, GhostPiece, TempPiece;
struct piece NxtPiece[5], HoldPiece, RotatePiece;

enum windowStates CurrentWindow;

void Initialize();
void functionTester();

void GamePlay();
void GamePause();
void GameOver();

void DrawGame();
void DrawBoard();
void DrawPiece();
void DrawNextPiece();

void PlayerControl();
void MoveX();
void Drop();
void Down();
void Swap();
void Rotate();

struct piece GetRandomPiece();
struct piece GetNewPiece(enum pieceShapes);
void MovePiece(struct piece*, int, int);
void MoveBlock(Vector2*, int, int);
void NextPiece();
void GetMainPiece(bool);
bool CheckBorderKicks();
void MoveGhostPiece();

bool CheckCollision(struct piece*);
void WhenCollideVertically();
void VerticalDrop();
void ClearBlock(int, int);
void ClearLines(int);
void isGameOver();
void ScoringSystem(enum pieceShapes, int);

int main()
{
    InitWindow(screenWidth, screenHeight, "New_Tetris.exe");
    SetTargetFPS(SETFPS);

    Initialize();

    while(!WindowShouldClose())
    {
        if(IsKeyPressed(KEY_R)) Initialize();

        BeginDrawing();
            ClearBackground(DARKGRAY);
            DrawGame();

            switch(CurrentWindow)
            {
                case WINDOWSTATE_PLAY: GamePlay(); break;
                case WINDOWSTATE_PAUSE: GamePause(); break;
                case WINDOWSTATE_GAMEOVER: GameOver(); break;
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void functionTester()
{

}

void Initialize()
{
    for(int h = 0; h < BOARDHEIGHT; h++)
    {
        for(int w = 0; w < BOARDWIDTH; w++)
        {
            Board[w][h] = (struct board){
                true, 
                {
                    (w + 6) * BOXSIZE,
                    (h + 0) * BOXSIZE
                },
                BLANK
            };
        }
    }

    CurrentWindow = WINDOWSTATE_PLAY;
    PieceRotation = 1;

    slow_Timer = (struct frameTimer){0, SETFPS / 2};
    drop_Timer = (struct frameTimer){0, SETFPS / 20};
    move_Timer = (struct frameTimer){0, SETFPS / 15};
    Stats = (struct gameStats){0, 0, 0};

    for(int i = 0; i < 5; i++)
        NxtPiece[i] = GetRandomPiece();
    NextPiece();
    HoldPiece = GetRandomPiece();
    NxtPiece[0] = GetRandomPiece();

    MainPiece = GetNewPiece(L_PIECE);
}

// ----- WINDOW-RELATED FUNCTIONS ----- //

void GamePlay()
{
    if(IsKeyPressed(KEY_P)) CurrentWindow = WINDOWSTATE_PAUSE;
    PlayerControl();
    MoveGhostPiece();    

    if(slow_Timer.counter >= slow_Timer.max)
    {
        VerticalDrop();
        slow_Timer.counter = 0;
    }
    slow_Timer.counter++;
    Stats.CurrentTimer++;
}

void GamePause()
{
    if(IsKeyPressed(KEY_P)) CurrentWindow = WINDOWSTATE_PLAY;
    DrawRectangle(BOXSIZE * 2, BOXSIZE, BOARDWIDTH * BOXSIZE, BOARDHEIGHT * BOXSIZE, (Color){80, 80, 80, 155});
    DrawRectangle(BOXSIZE, BOXSIZE * 2, BOXSIZE * 4, BOXSIZE * 4, (Color){80, 80, 80, 155});
    DrawRectangle((BOARDWIDTH + 7) * BOXSIZE, BOXSIZE * 2, BOXSIZE * 4, BOXSIZE * 15, (Color){80, 80, 80, 155});
}

void GameOver()
{
    DrawRectangle(BOXSIZE * 6, BOXSIZE, BOARDWIDTH * BOXSIZE, BOARDHEIGHT * BOXSIZE, (Color){80, 80, 80, 155});
}

// ----- DRAWING-RELATED FUNCTIONS ----- //

void DrawGame()
{
    DrawPiece();
    DrawBoard();
    DrawNextPiece();
}

void DrawBoard()
{
    for(int h = 1; h < BOARDHEIGHT; h++)
    {
        for(int w = 0; w < BOARDWIDTH; w++)
        {
            //It draw the block (this does not include the mainpiece)
            if(!Board[w][h].IsBlank)
                DrawRectangle(Board[w][h].Coords.x, Board[w][h].Coords.y, BOXSIZE, BOXSIZE, Board[w][h].color);
        }
    }

    DrawRectangleLines(BOXSIZE * 6, BOXSIZE, BOARDWIDTH * BOXSIZE, (BOARDHEIGHT - 1) * BOXSIZE, LIGHTGRAY);
    DrawRectangleLines(BOXSIZE, BOXSIZE * 2, BOXSIZE * 4, BOXSIZE * 4, LIGHTGRAY);
    DrawRectangleLines((BOARDWIDTH + 7) * BOXSIZE, BOXSIZE * 2, BOXSIZE * 4, BOXSIZE * 15, LIGHTGRAY);
}

void DrawPiece()
{
    for(int i = 0; i < 4; i++)
    {
        DrawRectangle(Board[(int)GhostPiece.Coords[i].x][(int)GhostPiece.Coords[i].y].Coords.x,
                      Board[(int)GhostPiece.Coords[i].x][(int)GhostPiece.Coords[i].y].Coords.y,
                      BOXSIZE, BOXSIZE, GhostPiece.color);
                      
        if(MainPiece.Coords[i].y > 0)
            DrawRectangle(Board[(int)MainPiece.Coords[i].x][(int)MainPiece.Coords[i].y].Coords.x,
                        Board[(int)MainPiece.Coords[i].x][(int)MainPiece.Coords[i].y].Coords.y,
                        BOXSIZE, BOXSIZE, MainPiece.color);
    }
}

void DrawNextPiece()
{
    for(int i = 0; i < 5; i++)
    {
        for(int n = 0; n < 4; n++)
        {
            DrawRectangle((BOARDWIDTH + 7 + NxtPiece[i].Coords[n].x) * BOXSIZE,
                          (2 + (3 * i) + NxtPiece[i].Coords[n].y) * BOXSIZE,
                           BOXSIZE, BOXSIZE, NxtPiece[i].color);
        }
    }

    for(int i = 0; i < 4; i++)
        DrawRectangle((HoldPiece.Coords[i].x + 1) * BOXSIZE,
                      (HoldPiece.Coords[i].y + 3) * BOXSIZE,
                       BOXSIZE, BOXSIZE, HoldPiece.color);
}

// ----- CONTROL-RELATED FUNCTIONS ----- //

void PlayerControl()
{
    //moving left and right
    if((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT)))
        MoveX();
    
    //drop and down
    if(IsKeyPressed(KEY_SPACE))
        Drop();

    if(IsKeyPressed(KEY_S))
    {
        slow_Timer.max = SETFPS / 20;
        drop_Timer.max = SETFPS / 2;
    }
    else if(IsKeyReleased(KEY_S))
    {
        slow_Timer.max = SETFPS / 2;
        drop_Timer.max = SETFPS / 20;
    }

    //rotate and swap
    if(IsKeyPressed(KEY_UP))
        Rotate();
    else if(IsKeyPressed(KEY_D) && CanPieceRotate)
        Swap();
}

void MoveX()
{
    move_Timer.counter++;
    if(move_Timer.counter >= move_Timer.max)
    {
        move_Timer.counter = 0;

        int temp = (IsKeyDown(KEY_LEFT))? -1: 1;

        MovePiece(&MainPiece, temp, 0);
        if(CheckCollision(&MainPiece))
            MovePiece(&MainPiece, -temp, 0);
    }

    drop_Timer.counter = 0;
}

void Drop()
{
    for(int i = 0; i < 4; i++)
        MainPiece.Coords[i] = GhostPiece.Coords[i];

    slow_Timer.counter = 0;
    drop_Timer.counter = 0;

    WhenCollideVertically();
}

void Down()
{
    
}

void Swap()
{
    drop_Timer.counter = 0;
    slow_Timer.counter = 0;

    TempPiece = HoldPiece;
    HoldPiece = GetNewPiece(MainPiece.shape);
    MainPiece = TempPiece;

    GetMainPiece(false);
}

void Rotate()
{
    RotatePiece = MainPiece;
    int tempX, tempY;

    PieceRotation = (PieceRotation > 4)? 1 : PieceRotation + 1;

    switch(RotatePiece.shape)
    {
        case O_PIECE: break;
        case I_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1 : -1;
            tempY = (PieceRotation <= 2)? 1 : -1;
                MoveBlock(&RotatePiece.Coords[0], -tempX, -tempY);
                MoveBlock(&RotatePiece.Coords[2], tempX, tempY);
                MoveBlock(&RotatePiece.Coords[3], tempX * 2, tempY * 2);
            break;

        case L_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1 : -1;
            tempY = (PieceRotation <= 2)? 1 : -1;
                MoveBlock(&RotatePiece.Coords[0], -tempX, -tempY);
                MoveBlock(&RotatePiece.Coords[2], tempX, tempY);
            if(PieceRotation % 2 == 0) MoveBlock(&RotatePiece.Coords[3], -tempY * 2, 0);
            else MoveBlock(&RotatePiece.Coords[3], 0, -tempY * 2);
            break;

        case RL_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1: -1;
            tempY = (PieceRotation <= 2)? 1 : -1;
                MoveBlock(&RotatePiece.Coords[0], tempX, tempY);
                MoveBlock(&RotatePiece.Coords[2], -tempX, -tempY);
            if(PieceRotation % 2 == 0) MoveBlock(&RotatePiece.Coords[3], 0, tempY * 2);
            else MoveBlock(&RotatePiece.Coords[3], -tempY * 2, 0);
            break;

        case S_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1 : -1;
            tempY = (PieceRotation <= 2)? -1 : 1;
                MoveBlock(&RotatePiece.Coords[0], tempX, tempY);
                MoveBlock(&RotatePiece.Coords[2], -tempY, tempX);
            if(PieceRotation % 2 == 0) MoveBlock(&RotatePiece.Coords[3], -tempY * 2, 0);
            else MoveBlock(&RotatePiece.Coords[3], 0, tempX * 2);
            break;

        case RS_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1 : -1;
            tempY = (PieceRotation <= 2)? -1 : 1;
                MoveBlock(&RotatePiece.Coords[0], -tempX, tempY);
                MoveBlock(&RotatePiece.Coords[2], tempY, tempX);
            if(PieceRotation % 2 == 0) MoveBlock(&RotatePiece.Coords[3], tempY * 2, 0);
            else MoveBlock(&RotatePiece.Coords[3], 0, tempX * 2);
            break;

        case T_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1 : -1;
            tempY = (PieceRotation <= 2)? -1 : 1;
                MoveBlock(&RotatePiece.Coords[0], -tempX, -tempY);
                MoveBlock(&RotatePiece.Coords[2], tempY, -tempX);
                MoveBlock(&RotatePiece.Coords[3], tempX, tempY);
            break;
    }

    if(CheckCollision(&RotatePiece))
    {
        if(CheckBorderKicks())
            for(int i = 0; i < 4; i++)
                MainPiece.Coords[i] = RotatePiece.Coords[i];
        else
            PieceRotation = (PieceRotation < 1)? 4 : PieceRotation - 1;
    }
    else
        for(int i = 0; i < 4; i++)
                MainPiece.Coords[i] = RotatePiece.Coords[i];
    drop_Timer.counter = 0;
}
//139808
// ----- PIECE-RELATED FUNCTIONS ----- //

struct piece GetRandomPiece()
{
    enum pieceShapes Shape = GetRandomValue(0, 6);
    return GetNewPiece(Shape);
}

struct piece GetNewPiece(enum pieceShapes Shape)
{
    struct piece NewPiece;
    NewPiece.shape = Shape;

    switch(NewPiece.shape)
    {
        case O_PIECE:
            NewPiece.Coords[0].x = 1; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 1; NewPiece.Coords[3].x = 2;
            NewPiece.Coords[0].y = 0; NewPiece.Coords[1].y = 0; NewPiece.Coords[2].y = 1; NewPiece.Coords[3].y = 1;  
            NewPiece.color = YELLOW;
            break;
        case I_PIECE:
            NewPiece.Coords[0].x = 0; NewPiece.Coords[1].x = 1; NewPiece.Coords[2].x = 2; NewPiece.Coords[3].x = 3;
            NewPiece.Coords[0].y = 0; NewPiece.Coords[1].y = 0; NewPiece.Coords[2].y = 0; NewPiece.Coords[3].y = 0;  
            NewPiece.color = SKYBLUE;
            break;
        case L_PIECE:
            NewPiece.Coords[0].x = 1; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 3; NewPiece.Coords[3].x = 3;
            NewPiece.Coords[0].y = 1; NewPiece.Coords[1].y = 1; NewPiece.Coords[2].y = 1; NewPiece.Coords[3].y = 0;  
            NewPiece.color = ORANGE;
            break;
        case RL_PIECE:
            NewPiece.Coords[0].x = 3; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 1; NewPiece.Coords[3].x = 1;
            NewPiece.Coords[0].y = 1; NewPiece.Coords[1].y = 1; NewPiece.Coords[2].y = 1; NewPiece.Coords[3].y = 0;  
            NewPiece.color = BLUE;
            break;
        case S_PIECE:
            NewPiece.Coords[0].x = 3; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 2; NewPiece.Coords[3].x = 1;
            NewPiece.Coords[0].y = 0; NewPiece.Coords[1].y = 0; NewPiece.Coords[2].y = 1; NewPiece.Coords[3].y = 1;  
            NewPiece.color = LIME;
            break;
        case RS_PIECE:
            NewPiece.Coords[0].x = 1; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 2; NewPiece.Coords[3].x = 3;
            NewPiece.Coords[0].y = 0; NewPiece.Coords[1].y = 0; NewPiece.Coords[2].y = 1; NewPiece.Coords[3].y = 1;  
            NewPiece.color = RED;
            break;
        case T_PIECE:
            NewPiece.Coords[0].x = 1; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 2; NewPiece.Coords[3].x = 3;
            NewPiece.Coords[0].y = 1; NewPiece.Coords[1].y = 1; NewPiece.Coords[2].y = 0; NewPiece.Coords[3].y = 1;  
            NewPiece.color = PURPLE;
            break;
        default: break;
    }

    return NewPiece;
}

void MovePiece(struct piece* Piece, int MovementX, int MovementY)
{
    for(int i = 0; i < 4; i++)
    {
        Piece->Coords[i].x += MovementX;
        Piece->Coords[i].y += MovementY;
    }
}

void MoveBlock(Vector2* Block, int MovementX, int MovementY)
{
    Block->x += MovementX;
    Block->y += MovementY;
}

void NextPiece()
{
    MainPiece = NxtPiece[0];
    for(int i = 0; i < 4; i++)
        NxtPiece[i] = NxtPiece[i + 1];
    NxtPiece[4] = GetRandomPiece();

    GetMainPiece(true);
    isGameOver();   
}

void GetMainPiece(bool canRotate)
{
    for(int i = 0; i < 4; i++)
        MainPiece.Coords[i].x += 3;
    CanPieceRotate = canRotate;
    GhostPiece = MainPiece;
    GhostPiece.color.a = 90;
}

bool CheckBorderKicks()
{
    int temp;

    for(int i = 0; i < 4; i++)
    {
        temp = (i % 2 == 0)? 1 : -1;
        if(i <= 1) MovePiece(&RotatePiece, temp, 0);
        else MovePiece(&RotatePiece, 0, temp);

        if(CheckCollision(&RotatePiece))
        {
            if(i <= 1) MovePiece(&RotatePiece, -temp, 0);
            else MovePiece(&RotatePiece, 0, -temp);
        }
        else return true;
    }

    return false;
}

void MoveGhostPiece()
{
    for(int i = 0; i < 4; i++)
        GhostPiece.Coords[i] = MainPiece.Coords[i];

    while(!CheckCollision(&GhostPiece))
        MovePiece(&GhostPiece, 0, 1);

    MovePiece(&GhostPiece, 0, -1);
}

// ----- PHYSICS-RELATED FUNCTIONS ----- //

bool CheckCollision(struct piece* Piece)
{
    for(int i = 0; i < 4; i++)
    {
        if(Piece->Coords[i].y >= BOARDHEIGHT ||
           Piece->Coords[i].x >= BOARDWIDTH ||
           Piece->Coords[i].x < 0           ||
           !Board[(int)Piece->Coords[i].x][(int)Piece->Coords[i].y].IsBlank           
          )

        return true;
    
    }

    return false;
}

void WhenCollideVertically()
{
    int lines = 0;
    bool IsLineClear = true;

    for(int i = 0; i < 4; i++)
    {
        Board[(int)MainPiece.Coords[i].x][(int)MainPiece.Coords[i].y].IsBlank = false;
        Board[(int)MainPiece.Coords[i].x][(int)MainPiece.Coords[i].y].color = MainPiece.color;
    }

    for(int h = 0; h < BOARDHEIGHT; h++)
    {
        for(int w = 0; w < BOARDWIDTH; w++)
            if(Board[w][h].IsBlank)
                IsLineClear = false;
        if(IsLineClear)
        {
            ClearLines(h);
            lines++;
        }
        IsLineClear = true;
    }

    ScoringSystem(MainPiece.shape, lines);
    PieceRotation = 1;
    NextPiece();
}

void ClearBlock(int w, int h)
{
    Board[w][h].IsBlank = true;
    Board[w][h].color = BLANK;
}

void ClearLines(int line)
{
    for(int w = 0; w < BOARDWIDTH; w++)
        ClearBlock(w, line);
    for(int h = line; h > 0; h--)
        for(int w = 0; w < BOARDWIDTH; w++)
        {
            Board[w][h].IsBlank = Board[w][h - 1].IsBlank;
            Board[w][h].color = Board[w][h-1].color;
        }
    for(int w = 0; w < BOARDWIDTH; w++)
        ClearBlock(w, 0);
}

void VerticalDrop()
{
    MovePiece(&MainPiece, 0, 1);

    if(CheckCollision(&MainPiece))
    {
        MovePiece(&MainPiece, 0, -1);
        //This allows piece movement after it dropped
        drop_Timer.counter++;
        if(drop_Timer.counter >= drop_Timer.max)
            WhenCollideVertically();
    }
}

void isGameOver()
{
    if(CheckCollision(&MainPiece))
    {
        CurrentWindow = WINDOWSTATE_GAMEOVER;

        for(int h = 0; h < BOARDHEIGHT; h++)
            for(int w = 0; w < BOARDWIDTH; w++)
                Board[w][h].color = GRAY;

        for(int i = 0; i < 5; i++)
            NxtPiece[i].color = GRAY;
        
        MainPiece.color = GRAY;
        HoldPiece.color = GRAY;
        GhostPiece.color = GRAY;
    }
}

void ScoringSystem(enum pieceShapes shape, int lines)
{
    float shapeScore = 0.0f;
    int maxLines = 0;

    switch(shape)
    {
        case O_PIECE:
            shapeScore = 6.0f;
            maxLines = 2;
            break;
        case I_PIECE:
            shapeScore = 8.0f;
            maxLines = 4;
            break;
        case L_PIECE:
            shapeScore = 7.5f;
            maxLines = 3;
            break;
        case RL_PIECE:
            shapeScore = 7.5f;
            maxLines = 3;
            break;
        case S_PIECE:
            shapeScore = 7.0f;
            maxLines = 2;
            break;
        case RS_PIECE:
            shapeScore = 7.0f;
            maxLines = 2;
            break;
        case T_PIECE:
            shapeScore = 6.5f;
            maxLines = 2;
            break;
    }

    if(maxLines == lines)
        shapeScore *= 2.0f;

    Stats.Lines += lines;
    Stats.Score += (lines * shapeScore) + (maxLines * (int)shape);
}
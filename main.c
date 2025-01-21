#include <stdio.h>
#include <stdlib.h>
#include <conio.h>  // 用於 getch() 和 kbhit()
#include <windows.h> // 用於 gotoxy() 和 Sleep()
//#include <process.h>

#define FRAME_WIDTH 40
#define FRAME_HEIGHT 20
#define MAX_BALLS 10

CRITICAL_SECTION cs;
int bar_x = FRAME_WIDTH / 2 - 1;// 條型的初始位置
int bar_y = FRAME_HEIGHT - 1;     // 條形的高度
int ball_x[MAX_BALLS];  // 球的 x 坐標 (-1 表示球未發射)
int ball_y[MAX_BALLS];  // 球的 y 坐標 (-1 表示球未發射)
int obstacle_x = -1;  // 障礙物的 x 坐標
int obstacle_y = 1;   // 障礙物的初始高度
int score = 0;        // 玩家分數
int game_running = 1;
int Heart = 3;

int ball_speed = 1;          // 控制球的速度（數字越大越慢）
int obstacle_speed = 5;      // 控制障礙物的速度（數字越大越慢）
int ball_speed_counter = 0;  // 球速度計數器
int obstacle_speed_counter = 0;  // 障礙物速度計數器

int MIN_OBSTACLE_SPEED = 1;  // 障礙物最小速度

FILE *log_file;  // 用於紀錄遊戲的檔案指標

// 清屏
void clrscr(){
    system("cls");
}

// 設定光標位置
void gotoxy(int x, int y){
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 延遲函數
void delay(int ms){
    Sleep(ms);
}

// 顯示分數
void display(){
	gotoxy(FRAME_WIDTH + 2, 1);
	printf("Press ESC to exit.");
    gotoxy(FRAME_WIDTH + 2, 2);
    printf("Score: %d", score);
    gotoxy(FRAME_WIDTH + 2, 3);
    printf("Heart: %d", Heart);
}

// 顯示框架
void drawFrame(){
    for (int i = 0; i <= FRAME_HEIGHT; i++){
        gotoxy(0, i);
        putchar('|');
        gotoxy(FRAME_WIDTH, i);
        putchar('|');
    }
    for (int i = 0; i <= FRAME_WIDTH; i++){
        gotoxy(i, FRAME_HEIGHT);
        putchar('-');
    }
}

void initializeBalls(){
    for (int i = 0; i < MAX_BALLS; i++){
        ball_x[i] = -1;  // 表示球未被發射
        ball_y[i] = -1;
    }
}

// 移動條形
void moveBar(char direction){
    gotoxy(bar_x, bar_y);
    printf("   ");  // 清除條形原來的位置

    if (direction == 'a' && bar_x > 1){
        bar_x--;  // 向左移動
    }
	else if (direction == 'd' && bar_x < FRAME_WIDTH - 3){
        bar_x++;  // 向右移動
    }

    gotoxy(bar_x, bar_y);
    printf("===");  // 顯示條形
}

// 發射球
void shootBall(){
    for (int i = 0; i < MAX_BALLS; i++){
        if (ball_x[i] == -1 && ball_y[i] == -1){  // 找到一個未被使用的球
            ball_x[i] = bar_x + 1;  // 球從條形中間發射
            ball_y[i] = bar_y - 1;
            break;
        }
    }
}

// 更新球的位置
void updateBalls(){
    for (int i = 0; i < MAX_BALLS; i++){
        if (ball_x[i] != -1 && ball_y[i] != -1){  // 如果球已經被發射
            gotoxy(ball_x[i], ball_y[i]);
            putchar(' ');  // 清除原來的位置

            ball_y[i]--;  // 球向上移動

            if (ball_y[i] < 0){  // 如果球到達頂部
                ball_x[i] = -1;  // 重置球
                ball_y[i] = -1;
            }
			else{
                gotoxy(ball_x[i], ball_y[i]);
                putchar('*');  // 顯示新的球位置
            }
        }
    }
}

// 生成障礙物
void generateObstacle(){
    if (obstacle_x == -1){  // 如果沒有障礙物
        obstacle_x = 2 + rand() % (FRAME_WIDTH - 3);  // 隨機生成障礙物的 x 坐標
    }
    gotoxy(obstacle_x, obstacle_y);
    putchar('O');  // 顯示障礙物
}

// 更新障礙物的位置
void updateObstacle(){
    if (obstacle_x != -1){
        gotoxy(obstacle_x, obstacle_y);
        putchar(' ');  // 清除障礙物的舊位置

        obstacle_y++;  // 障礙物向下移動

        if (obstacle_y > FRAME_HEIGHT){  // 如果障礙物到達底部
            obstacle_x = -1;  // 重置障礙物位置
            obstacle_y = 2;
        }
		else{
            gotoxy(obstacle_x, obstacle_y);
            putchar('O');  // 顯示障礙物的新位置
        }
    }
    
}

// 調整障礙物速度
void adjustObstacleSpeed(){
    if (obstacle_speed > MIN_OBSTACLE_SPEED && score % 5 == 0) {  // 每得 5 分加速一次
        obstacle_speed--;
    }
}

// 檢查碰撞
void checkCollision(){
    for (int i = 0; i < MAX_BALLS; i++){
        if (ball_x[i] == obstacle_x && ball_y[i] == obstacle_y){
            score++;
            display();
            adjustObstacleSpeed();

            // 重置球和障礙物
            ball_x[i] = -1;
            ball_y[i] = -1;
            obstacle_x = -1;
            obstacle_y = 2;
        }
    }
}

void checkBottom(){
	if (obstacle_y == 20){
		Heart--;
		display();
	}
}
// 主遊戲邏輯
void gameLoop(){
    char input,ch;
    
    while (game_running){
    	drawFrame();
    	if (Heart == 0){
            	game_running = 0;
            	break;
		}
        if (kbhit()){  // 檢測鍵盤輸入
            input = getch();
            if (input == 27){
            	game_running = 0;
            	break;
			}
            else if (input == 'a' || input == 'd'){
                moveBar(input);  // 控制條形移動
            }
			else if (input == 'w'){
                shootBall();  // 發射球
            }
        }
        
        // 控制球的速度
        ball_speed_counter++;
        if (ball_speed_counter >= ball_speed) {
            updateBalls();
            ball_speed_counter = 0;
        }

        // 控制障礙物的速度
        obstacle_speed_counter++;
        if (obstacle_speed_counter >= obstacle_speed) {
            updateObstacle();
            obstacle_speed_counter = 0;
            checkBottom();
        }
		generateObstacle();
//        updateBalls();       // 更新球的位置
//        updateObstacle();   // 更新障礙物的位置
        checkCollision();
        
		delay(100);
    }
}

void logGameResult() {
    // 獲取當前時間
    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';  // 去掉換行符

    // 紀錄到檔案
    fprintf(log_file, "Game Over! Final Score: %d, Time: %s\n", score, time_str);
}

int main() {
    srand(time(NULL));  // 初始化隨機數
    
    // 開啟紀錄檔案
    log_file = fopen("game_log.txt", "a");
    if (!log_file) {
        printf("Error opening log file.\n");
        return 1;
    }

    clrscr();
    drawFrame();
    display();
    initializeBalls();

    gameLoop();  // 開始遊戲
    
    logGameResult();  // 紀錄遊戲結果

    fclose(log_file);  // 關閉紀錄檔案
    
    clrscr();
    gotoxy(10, 10);
    printf("Game Over! Final Score: %d\n", score);

    return 0;
}

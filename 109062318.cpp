#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void write_valid_spot(std::ofstream& fout) {
    int n_valid_spots = next_valid_spots.size();
    srand(time(NULL));
    // Choose random spot. (Not random uniform here)
    int index = (rand() % n_valid_spots);
    Point p = next_valid_spots[index];
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}
class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;
private:
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard(std::array<std::array<int, SIZE>, SIZE> new_board, int player) {
         board = new_board;
         cur_player = player;
         next_valid_spots = get_valid_spots();
         done = false;
         for(int i = 0 ; i < 8 ; i++){
            for(int q = 0 ; q < 8 ; q++){
                disc_count[board[i][q]]++;
            }
         }
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return true;
    }
};

int value(OthelloBoard now,int player){
    int i,j;
    int heuristic=0;
    int mytile=0,opptile=0;

    //棋子數量
    for(i=0;i<8;i++){
        for(j=0;j<8;j++){
            if(now.board[i][j] == player){
                mytile++;
            }else if(now.board[i][j] == 3-player){
                opptile++;
            }
        }
    }
    heuristic += 10*(mytile-opptile);
    //std::cout<<"mytile="<<mytile<<",opptile="<<opptile<<"\n";

    //角落
    Point corner[4]={Point(0,0),Point(0,7),Point(7,0),Point(7,7)};
    for(auto p:corner){
        if(now.board[p.x][p.y]==player){
            heuristic+=1e4;
        }else if(now.board[p.x][p.y]==3-player){
            heuristic-=1e4;
        }
    }

    //角落旁的點
    Point bad[4]={Point(1,1),Point(1,6),Point(6,1),Point(6,6)};
    for(auto p:bad){
        if(now.board[p.x][p.y]==player){
            heuristic-=1e4;
        }else if(now.board[p.x][p.y]==3-player){
            heuristic+=1e4;
        }
    }
    //std::cout<<"heuristic="<<heuristic<<"\n";
    return heuristic;
}
//maxdepth(=5)可以寫在if中，就不需要當作參數傳入了
int alphabeta(OthelloBoard now,int depth/*,int maxdepth*/,int alpha,int beta,bool minmax){
    int i;
    if(depth==5){
        return value(now,player);
    }
    if(minmax){ //on player node
        int nowval=-1e9;
        for(i=0;i < (int)now.next_valid_spots.size();i++){
            OthelloBoard next = now;
            next.put_disc(now.next_valid_spots[i]);
            nowval = std::max(nowval , alphabeta(next,depth+1,alpha,beta,false/*!minmax*/));
            alpha = std::max(alpha , nowval);
            if(alpha>=beta){
                break;
            }
        }
        return nowval;
    }else{  //on opponent node
        int nowval=1e9;
        for(i=0;i < (int)now.next_valid_spots.size();i++){
            OthelloBoard next = now;
            next.put_disc(now.next_valid_spots[i]);
            nowval = std::min(nowval , alphabeta(next,depth+1,alpha,beta,true/*!minmax*/));
            beta = std::min(beta , nowval);
            if(alpha>=beta){
                break;
            }
        }
        return nowval;
    }
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);

    OthelloBoard now(board,player);
    alphabeta(now,0,-1e9,1e9,true);
    
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
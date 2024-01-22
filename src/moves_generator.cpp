#include "moves_generator.h"
#include "chess_types.h"
#include "game.h"
#include <iostream>
#include <unordered_map>
#include <vector>


inline void setBit(Bitboard &bitboard, int index){
    bitboard = bitboard | (1ULL << index);
}

inline Bitboard NOT(Bitboard bitboard){
    return ~bitboard;
}

inline int countBits(Bitboard bb){
    int count = 0;
    while(bb){
        count++;
        bb &= bb - 1;
    }
    return count;
}

inline Bitboard magicHash(Bitboard bb, Bitboard magic, int shift){
    return (bb * magic) >> (64-shift);
}

Bitboard randomU64(){
    Bitboard random = 0;
    for(int i = 0; i < 64; i++){
        if(rand() % 2 == 0)
            setBit(random, i);
    }
    return random;
}

std::list<Move> MovesGenerator::generateMoves(const Game& game){
    std::list<Move> moves;
    generatePawnMoves(game, moves);
    generateKnightMoves(game, moves);
    generateBishopMoves(game, moves);
    generateRookMoves(game, moves);
    generateQueenMoves(game, moves);
    generateKingMoves(game, moves);

    return moves;
}

MovesGenerator::MovesGenerator(){
    inizializeKnightPatterns();
    inizializeKingPatterns();
    inizializeRookMasks();
    inizializeBishopMasks();

    setRookMagicBitboard();
    setBishopMagicBitboard();
}

void MovesGenerator::inizializeRookMasks(){
    Bitboard files[8];
    Bitboard ranks[8];

    for(int i=0;i<8;i++){
        files[i] = 0ULL;
        ranks[i] = 0ULL;
    }

    for(int i=0;i<8;i++){
        if(i==0){
            files[i] = 0x8080808080808080ULL;
            ranks[i] = 0xFFULL;
        } else {
            files[i] = files[i-1] >> 1;
            ranks[i] = ranks[i-1] << 8;
        }
    }

    for(int i=0;i<64;i++){
        Bitboard mask = 0ULL;
        int file = 7 - i % 8;
        int rank = i / 8;
    
        ROOK_MASKS[i] = (files[file] | ranks[rank]);
        
        if(file != 0)
            ROOK_MASKS[i] &= ~FILE_A;
        if(file != 7)
            ROOK_MASKS[i] &= ~FILE_H;
        if(rank != 0)
            ROOK_MASKS[i] &= ~RANK_1;
        if(rank != 7)
            ROOK_MASKS[i] &= ~RANK_8;
        
        ROOK_MASKS[i] &= NOT(1ULL << i);
    }
}

void MovesGenerator::inizializeBishopMasks(){
    for(int i=0;i<64;i++){
        Bitboard mask = 0ULL;
        Bitboard index = 1ULL << i;
        Bitboard indexCopy = index;
        for(int j = 0; j < 8; j++){
            mask |= index;
            index = index & NOT(FILE_H);
            index = index << 7;
        }
        index = 1ULL << i;
        for(int j = 0; j < 8; j++){
            mask |= index;
            index = index & NOT(FILE_A);
            index = index << 9;
        }
        index = 1ULL << i;
        for(int j = 0; j < 8; j++){
            mask |= index;
            index = index & NOT(FILE_A);
            index = index >> 7;
        }
        index = 1ULL << i;
        for(int j = 0; j < 8; j++){
            mask |= index;
            index = index & NOT(FILE_H);
            index = index >> 9;
        }
        BISHOP_MASKS[i] = mask & NOT(indexCopy) & NOT(FILE_A) & NOT(FILE_H) & NOT(RANK_1) & NOT(RANK_8);
    }
}

void MovesGenerator::generateBishopMoves(const Game& game, std::list<Move> &moves){
        if(game.turn == WHITE){
        Bitboard white_bishops = game.pieces[WHITE][BISHOP];
        while(white_bishops){
            Bitboard white_bishop = white_bishops & -white_bishops;
            int index = getBitIndex(white_bishop);

            Bitboard white_bishop_moves = bishopDB[index][magicHash(game.all & BISHOP_MASKS[index], BISHOP_MAGIC_NUMBERS[index], BISHOP_SHIFT_BITS)] & ~game.occupied[WHITE];
            while(white_bishop_moves){
                Move move;
                move.color = WHITE;
                move.piece_type = BISHOP;
                move.to = white_bishop_moves & -white_bishop_moves;
                move.from = white_bishop;
                move.capture = game.occupied[BLACK] & move.to;
                move.en_passant = false;
                move.castling = false;
                move.is_promotion = false;

                if(isLegalMove(game, move)){
                    moves.push_back(move);
                }
                white_bishop_moves &= white_bishop_moves - 1;
            }
            white_bishops &= white_bishops - 1;
        }
    } else if(game.turn == BLACK){
        Bitboard black_bishops = game.pieces[BLACK][BISHOP];
        while(black_bishops){
            Bitboard black_bishop = black_bishops & -black_bishops;
            int index = getBitIndex(black_bishop);

            Bitboard black_bishop_moves = bishopDB[index][magicHash(game.all & BISHOP_MASKS[index], BISHOP_MAGIC_NUMBERS[index], BISHOP_SHIFT_BITS)] & ~game.occupied[BLACK];
            while(black_bishop_moves){
                Move move;
                move.color = BLACK;
                move.piece_type = BISHOP;
                move.to = black_bishop_moves & -black_bishop_moves;
                move.from = black_bishop;
                move.capture = game.occupied[WHITE] & move.to;
                move.en_passant = false;
                move.castling = false;
                move.is_promotion = false;

                if(isLegalMove(game, move)){
                    moves.push_back(move);
                }
                black_bishop_moves &= black_bishop_moves - 1;
            }
            black_bishops &= black_bishops - 1;
        }
    }
}

void MovesGenerator::generateRookMoves(const Game& game, std::list<Move> &moves){
    
    if(game.turn == WHITE){
        Bitboard white_rooks = game.pieces[WHITE][ROOK];
        while(white_rooks){
            Bitboard white_rook = white_rooks & -white_rooks;
            int index = getBitIndex(white_rook);

            Bitboard white_rook_moves = rookDB[index][magicHash(game.all & ROOK_MASKS[index],ROOK_MAGIC_NUMBERS[index], ROOK_SHIFT_BITS)] & ~game.occupied[WHITE];
        
            while(white_rook_moves){
                Move move;
                move.color = WHITE;
                move.piece_type = ROOK;
                move.to = white_rook_moves & -white_rook_moves;
                move.from = white_rook;
                move.capture = game.occupied[BLACK] & move.to;
                move.en_passant = false;
                move.castling = false;
                move.is_promotion = false;

                if(isLegalMove(game, move)){
                    moves.push_back(move);
                }
                white_rook_moves &= white_rook_moves - 1;
            }
            white_rooks &= white_rooks - 1;
        }
    } else if(game.turn == BLACK){
        Bitboard black_rooks = game.pieces[BLACK][ROOK];
        while(black_rooks){
            Bitboard black_rook = black_rooks & -black_rooks;
            int index = getBitIndex(black_rook);

            Bitboard black_rook_moves = rookDB[index][magicHash(game.all & ROOK_MASKS[index],ROOK_MAGIC_NUMBERS[index], ROOK_SHIFT_BITS)] & ~game.occupied[BLACK];
            while(black_rook_moves){
                Move move;
                move.color = BLACK;
                move.piece_type = ROOK;
                move.to = black_rook_moves & -black_rook_moves;
                move.from = black_rook;
                move.capture = game.occupied[WHITE] & move.to;
                move.en_passant = false;
                move.castling = false;
                move.is_promotion = false;

                if(isLegalMove(game, move)){
                    moves.push_back(move);
                }
                black_rook_moves &= black_rook_moves - 1;
            }
            black_rooks &= black_rooks - 1;
        }
    }
    
}

void MovesGenerator::generateQueenMoves(const Game& game, std::list<Move> &moves){
        if(game.turn == WHITE){
        Bitboard white_queens = game.pieces[WHITE][QUEEN];
        while(white_queens){
            Bitboard white_queen = white_queens & -white_queens;
            int index = getBitIndex(white_queen);
            
            Bitboard white_queen_moves = 
                (rookDB[index][magicHash(game.all & ROOK_MASKS[index], ROOK_MAGIC_NUMBERS[index], ROOK_SHIFT_BITS)] |
                bishopDB[index][magicHash(game.all & BISHOP_MASKS[index], BISHOP_MAGIC_NUMBERS[index], BISHOP_SHIFT_BITS)]) & 
                ~game.occupied[WHITE];

            while(white_queen_moves){
                Move move;
                move.color = WHITE;
                move.piece_type = QUEEN;
                move.to = white_queen_moves & -white_queen_moves;
                move.from = white_queen;
                move.capture = game.occupied[BLACK] & move.to;
                move.en_passant = false;
                move.castling = false;
                move.is_promotion = false;

                if(isLegalMove(game, move)){
                    moves.push_back(move);
                }
                white_queen_moves &= white_queen_moves - 1;
            }
            white_queens &= white_queens - 1;
        }
    } else if(game.turn == BLACK){
        Bitboard black_queens = game.pieces[BLACK][QUEEN];
        while(black_queens){
            Bitboard black_queen = black_queens & -black_queens;
            int index = getBitIndex(black_queen);

            Bitboard black_queen_moves = 
                (rookDB[index][magicHash(game.all & ROOK_MASKS[index], ROOK_MAGIC_NUMBERS[index], ROOK_SHIFT_BITS)] |
                bishopDB[index][magicHash(game.all & BISHOP_MASKS[index], BISHOP_MAGIC_NUMBERS[index], BISHOP_SHIFT_BITS)]) & 
                ~game.occupied[BLACK];

            while(black_queen_moves){
                Move move;
                move.color = BLACK;
                move.piece_type = QUEEN;
                move.to = black_queen_moves & -black_queen_moves;
                move.from = black_queen;
                move.capture = game.occupied[WHITE] & move.to;
                move.en_passant = false;
                move.castling = false;
                move.is_promotion = false;

                if(isLegalMove(game, move)){
                    moves.push_back(move);
                }
                black_queen_moves &= black_queen_moves - 1;
            }
            black_queens &= black_queens - 1;
        }
    }
}

bool MovesGenerator::isLegalMove(const Game& game, Move move){
    return true;
}

void MovesGenerator::setRookMagicBitboard(){
    std::unordered_map<Bitboard, std::vector<Bitboard>> rook_blockers;
    std::unordered_map<Bitboard, std::vector<Bitboard>> rook_attacks;

    for(int i = 0; i < 64; i++){
        Bitboard mask = ROOK_MASKS[i];
        int num_blockers = 1 << countBits(mask);


        for(int j = 0; j < num_blockers; j++){
            Bitboard blockers = 0;
            Bitboard index = j;
            mask = ROOK_MASKS[i];
            for(int k=0; k < 64 && index != 0; k++){
                if(index & 1){
                    blockers |= mask & -mask;
                }
                mask &= mask - 1;
                index >>= 1;
            }
            rook_blockers[i].push_back(blockers);
        }
    }
    
    for(int i = 0; i < 64; i++){
        Bitboard pos = 1ULL << i;
        std::vector<Bitboard> blockers = rook_blockers[i];
        for(int j = 0; j < blockers.size(); j++){
            Bitboard blocker = blockers[j];
            // calculate rooks attacks for this blocker
            Bitboard attacks = 0;
            Bitboard temp = pos;
            while(temp){
                temp &= NOT(FILE_A);
                temp <<= 1;
                attacks |= temp;
                temp &= NOT(blocker);
            }
            temp = pos;
            while(temp){
                temp &= NOT(FILE_H);
                temp >>= 1;
                attacks |= temp;
                temp &= NOT(blocker);
            }
            temp = pos;
            while(temp){
                temp &= NOT(RANK_8);
                temp <<= 8;
                attacks |= temp;
                temp &= NOT(blocker);
            }
            temp = pos;
            while(temp){
                temp &= NOT(RANK_1);
                temp >>= 8;
                attacks |= temp;
                temp &= NOT(blocker);
            }
            rook_attacks[i].push_back(attacks);  
        }
    }
    
    int s = 14;
    ROOK_SHIFT_BITS = s;
    rookDB = new Bitboard*[64]; 
    for(int i = 0; i < 64; i++){
        rookDB[i] = new Bitboard[1ULL << s];
        
    }

    // rookDB[i][magicHash(blockers, magic[i], 12)]

    for(int i=0;i<64;i++)
        ROOK_MAGIC_NUMBERS[i] = 0ULL;
            
    for(int i=0;i<64;i++){
        bool ok = false;
        
        while(!ok){
            Bitboard magic = randomU64();
            bool* used = new bool[1ULL << s];
            for(int j=0;j<(1ULL << s);j++){
                used[j] = false;
            } 
            std::vector<Bitboard> blockers = rook_blockers[i];
            bool magic_good = true;
            for(int j=0;j<blockers.size() && magic_good == true;j++){
                int index = magicHash(blockers[j], magic, s);
                if(!used[index]){
                    used[index] = true;
                    rookDB[i][index] = rook_attacks[i][j];
                } else if(rookDB[i][index] != rook_attacks[i][j]){
                    magic_good = false;
                }
            }
            if(magic_good){
                ok = true;
                ROOK_MAGIC_NUMBERS[i] = magic;
            }
        }
    }  
    
}

void MovesGenerator::setBishopMagicBitboard(){
    std::unordered_map<Bitboard, std::vector<Bitboard>> bishop_blockers;
    std::unordered_map<Bitboard, std::vector<Bitboard>> bishop_attacks;

    for(int i = 0; i < 64; i++){
        Bitboard mask = BISHOP_MASKS[i];
        int num_blockers = 1 << countBits(mask);

        for(int j=0; j < num_blockers; j++){
            Bitboard blockers = 0;
            Bitboard index = j;
            mask = BISHOP_MASKS[i];
            for(int k=0; k < 64 && index != 0; k++){
                if(index & 1){
                    blockers |= mask & -mask;
                }
                mask &= mask - 1;
                index >>= 1;
            }
            bishop_blockers[i].push_back(blockers);
        }
    }

    for(int i = 0; i < 64; i++){
        Bitboard pos = 1ULL << i;
        std::vector<Bitboard> blockers = bishop_blockers[i];
        for(int j = 0; j < blockers.size(); j++){
            Bitboard blocker = blockers[j];
            // calculate bishops attacks for this blocker
            Bitboard attacks = 0;
            Bitboard temp = pos;
            while(temp){
                temp &= NOT(FILE_H);
                temp <<= 7;
                attacks |= temp;
                temp &= NOT(blocker);
            }
            temp = pos;
            while(temp){
                temp &= NOT(FILE_A);
                temp <<= 9;
                attacks |= temp;
                temp &= NOT(blocker);
            }
            temp = pos;
            while(temp){
                temp &= NOT(FILE_H);
                temp >>= 9;
                attacks |= temp;
                temp &= NOT(blocker);
            }
            temp = pos;
            while(temp){
                temp &= NOT(FILE_A);
                temp >>= 7;
                attacks |= temp;
                temp &= NOT(blocker);
            }
            bishop_attacks[i].push_back(attacks);  
        }
    }

    int s = 13;
    BISHOP_SHIFT_BITS = s;
    bishopDB = new Bitboard*[64];
    for(int i = 0; i < 64; i++){
        bishopDB[i] = new Bitboard[1ULL << s];
    }

    // bishopDB[i][magicHash(blockers, magic[i], 9)]

    for(int i=0;i<64;i++)
        BISHOP_MAGIC_NUMBERS[i] = 0ULL;

    for(int i=0;i<64;i++){
        bool ok = false;

        while(!ok){
            Bitboard magic = randomU64();
            bool* used = new bool[1ULL << s];
            for(int j=0;j<(1ULL << s);j++){
                used[j] = false;
            } 
            std::vector<Bitboard> blockers = bishop_blockers[i];
            bool magic_good = true;
            for(int j=0;j<blockers.size() && magic_good == true;j++){
                int index = magicHash(blockers[j], magic, s);
                if(!used[index]){
                    used[index] = true;
                    bishopDB[i][index] = bishop_attacks[i][j];
                } else if(bishopDB[i][index] != bishop_attacks[i][j]){
                    magic_good = false;
                }
            }
            if(magic_good){
                ok = true;
                BISHOP_MAGIC_NUMBERS[i] = magic;
            }
        }
    }
}

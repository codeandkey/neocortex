#include "eval.h"

#include <assert.h>
#include <string.h>

int NC_MATERIAL_MG[] =
{
	100, -100,
	300, -300,
	300, -300,
	500, -500,
	900, -900,
	1200, -1200
};

int NC_MATERIAL_EG[] =
{
	100, -100,
	300, -300,
	300, -300,
	500, -500,
	900, -900,
	1200, -1200
};

int NC_GUARD[] =
{
	9, -9,
	6, -6,
	5, -5,
	2, -2,
	1, -1,
	1, -1
};

int NC_EVAL_PHASE_VALS[] = 
{
    0, 0,
    1, 1, 
    1, 1, 
    2, 2, 
    4, 4,
    0, 0,
};

int NC_EVAL_PHASE_TOTAL = 32;

int NC_EVAL_CENTER_CONTROL_MG  = 20;
int NC_EVAL_CENTER_CONTROL_EG  = 8;
int NC_EVAL_KING_SAFETY_MG     = 7;
int NC_EVAL_KING_SAFETY_EG     = 6;
int NC_EVAL_PASSED_PAWNS_MG    = 15;
int NC_EVAL_PASSED_PAWNS_EG    = 30;
int NC_EVAL_ADV_PASSEDPAWN_MG  = 8;
int NC_EVAL_ADV_PASSEDPAWN_EG  = 8;
int NC_EVAL_DEVELOPMENT_MG     = 35;
int NC_EVAL_DEVELOPMENT_EG     = 20;
int NC_EVAL_FIRST_RANK_KING_MG = 10;
int NC_EVAL_FIRST_RANK_KING_EG = -10;
int NC_EVAL_PAWNS_PROT_KING_MG = 8;
int NC_EVAL_PAWNS_PROT_KING_EG = 8;
int NC_EVAL_EDGE_KNIGHTS_MG    = -10;
int NC_EVAL_EDGE_KNIGHTS_EG    = -5;
int NC_EVAL_ISOLATED_PAWNS_MG  = -10;
int NC_EVAL_ISOLATED_PAWNS_EG  = -10;
int NC_EVAL_BACKWARD_PAWNS_MG  = -10;
int NC_EVAL_BACKWARD_PAWNS_EG  = -10;
int NC_EVAL_DOUBLED_PAWNS_MG   = -10;
int NC_EVAL_DOUBLED_PAWNS_EG   = -20;
int NC_EVAL_PAWN_CHAIN_MG      = 4;
int NC_EVAL_PAWN_CHAIN_EG      = 4;
int NC_EVAL_OPEN_FILE_ROOK_MG  = 5;
int NC_EVAL_OPEN_FILE_ROOK_EG  = 5;
int NC_EVAL_OPEN_FILE_QUEEN_MG = 5;
int NC_EVAL_OPEN_FILE_QUEEN_EG = 5;

typedef struct {
    const char* name;
    int* ptr;
} ncEvalOption;

static ncEvalOption options[] = {
    { "center_mg", &NC_EVAL_CENTER_CONTROL_MG },
    { "king_safety_mg", &NC_EVAL_KING_SAFETY_MG },
    { "passers_mg", &NC_EVAL_PASSED_PAWNS_MG },
    { "advpassers_mg", &NC_EVAL_ADV_PASSEDPAWN_MG },
    { "development_mg", &NC_EVAL_DEVELOPMENT_MG },
    { "first_rank_king_mg", &NC_EVAL_FIRST_RANK_KING_MG },
    { "pawns_prot_king_mg", &NC_EVAL_PAWNS_PROT_KING_MG },
    { "edge_knights_mg", &NC_EVAL_EDGE_KNIGHTS_MG },
    { "isopawns_mg", &NC_EVAL_ISOLATED_PAWNS_MG },
    { "bckpawns_mg", &NC_EVAL_BACKWARD_PAWNS_MG },
    { "dblpawns_mg", &NC_EVAL_DOUBLED_PAWNS_MG },
    { "chnpawns_mg", &NC_EVAL_PAWN_CHAIN_MG },
    { "openfilerook_mg", &NC_EVAL_OPEN_FILE_ROOK_MG },
    { "openfilequeen_mg", &NC_EVAL_OPEN_FILE_QUEEN_MG },
    { "center_eg", &NC_EVAL_CENTER_CONTROL_EG },
    { "king_safety_eg", &NC_EVAL_KING_SAFETY_EG },
    { "passers_eg", &NC_EVAL_PASSED_PAWNS_EG },
    { "advpassers_eg", &NC_EVAL_ADV_PASSEDPAWN_EG },
    { "development_eg", &NC_EVAL_DEVELOPMENT_EG },
    { "first_rank_king_eg", &NC_EVAL_FIRST_RANK_KING_EG },
    { "pawns_prot_king_eg", &NC_EVAL_PAWNS_PROT_KING_EG },
    { "edge_knights_eg", &NC_EVAL_EDGE_KNIGHTS_EG },
    { "isopawns_eg", &NC_EVAL_ISOLATED_PAWNS_EG },
    { "bckpawns_eg", &NC_EVAL_BACKWARD_PAWNS_EG },
    { "dblpawns_eg", &NC_EVAL_DOUBLED_PAWNS_EG },
    { "chnpawns_eg", &NC_EVAL_PAWN_CHAIN_EG },
    { "openfilerook_eg", &NC_EVAL_OPEN_FILE_ROOK_EG },
    { "openfilequeen_eg", &NC_EVAL_OPEN_FILE_QUEEN_EG },
};

int ncEvalNumOptions()
{
    return sizeof(options) / sizeof(options[0]);
}

const char* ncEvalOptionStr(int ind)
{
    assert(ind >= 0 && ind < ncEvalNumOptions());
    return options[ind].name;
}

int ncEvalSetOption(const char* name, int value)
{
    for (int i = 0; i < ncEvalNumOptions(); ++i)
    {
        if (!strcmp(name, options[i].name))
        {
            *(options[i].ptr) = value;
            return 0;
        }
    }

    return -1;
}

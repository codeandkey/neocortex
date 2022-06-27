#include "uci.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "position.h"
#include "search.h"

void ncUciPosition(char* arg, char* context);
void ncUciGo(char* arg, char* context);

void ncUciBestmove(ncMove move);
void ncUciInfo(ncSearchInfo info);

int ncUciStart()
{
    char* context, *command;
    char line[512];

    setbuf(stdout, NULL);

    fgets(line, sizeof(line) - 1, stdin);

    if (strcmp("uci\n", line))
    {
        fprintf(stderr, "error: expected 'uci'\n");
        return -1;
    }

    printf("id name neocortex 2.0\n");
    printf("id author Justin Stanley <jtst@iastate.edu>\n");
    printf("info string GLHF!\n");
    printf("uciok\n");
    printf("isready\n");

    ncPosition p;
    ncPositionInit(&p);
    ncSearchLoad(&p);

    while (1)
    {
        fgets(line, sizeof(line) - 1, stdin);

        command = strtok_r(line, " \t\n", &context);

        if (!command)
            continue;

        char* arg = strtok_r(NULL, " \t\n", &context);

        if (!strcmp("isready", command))
            printf("readyok\n");
        else if (!strcmp("quit", command))
            break;
        else if (!strcmp("position", command))
            ncUciPosition(arg, context);
        else if (!strcmp("go", command))
            ncUciGo(arg, context);
        else if (!strcmp("stop", command))
            ncSearchStop();
        else
        {
            printf("info string WARNING: unrecognized UCI command '%s'\n", command);
            continue;
        }
    }

    fprintf(stderr, "Bye!\n");
    return 0;
}

void ncUciPosition(char* arg, char* context)
{
    ncPosition pos;

    if (!arg)
    {
        printf("info string ERROR: position: missing 'fen' or 'startpos'\n");
        return;
    }

    if (!strcmp("fen", arg))
    {
        char* infen = strtok_r(NULL, "\r\n", &context);

        if (!infen)
        {
            printf("info string ERROR: position: fen: expected fen\n");
            return;
        }

        if (ncPositionFromFen(&pos, infen))
        {
            printf("info string ERROR: position: fen: invalid fen\n");
            return;
        }
    }

    if (strcmp("startpos", arg))
    {
        printf("info string ERROR: position: expected 'fen' or 'startpos'\n");
        return;
    }

    ncPositionInit(&pos);

    arg = strtok_r(NULL, " \t\n\r", &context);

    if (!arg)
    {
        ncSearchLoad(&pos);
        return;
    }

    if (strcmp("moves", arg))
    {
        printf("info string ERROR: position: expected 'moves'\n");
        return;
    }

    while ((arg = strtok_r(NULL, " \t\n\r", &context)))
    {
        ncMove nextmove = ncMoveFromUci(arg);

        char uci[6];
        ncMoveUCI(nextmove, uci);
        printf("info string Pushing %s\n", uci);

        if (!ncMoveValid(nextmove))
        {
            printf("info string ERROR: position: invalid move '%s'\n", arg);
            continue;
        }

        if (!ncPositionMakeMove(&pos, nextmove))
        {
            char posfen[100];
            ncPositionUnmakeMove(&pos);
            ncPositionToFen(&pos, posfen, sizeof(posfen));
            printf("info string ERROR: position '%s': illegal move '%s'", posfen, uci);
            continue;
        }
    }

    ncSearchLoad(&pos);
}

void ncUciGo(char* arg, char* context)
{
    int nodes = 50000000;
    int movetime = -1;

    while (arg)
    {
        if (!strcmp("nodes", arg))
        {
            arg = strtok_r(NULL, " \t\r\n", &context);

            if (!arg)
            {
                printf("info string ERROR go: nodes: expected argument\n");
                return;
            }
            
            nodes = strtol(arg, NULL, 10);
            arg = strtok_r(NULL, " \t\r\n", &context);
        } else if (!strcmp("movetime", arg))
        {
            arg = strtok_r(NULL, " \t\r\n", &context);

            if (!arg)
            {
                printf("info string ERROR go: movetime: expected argument\n");
                return;
            }
            
            movetime = strtol(arg, NULL, 10);
            arg = strtok_r(NULL, " \t\r\n", &context);
        } else
            printf("info string ERROR go: unknown argument '%s'\n", arg);
    }

    ncSearchStart(nodes, movetime, ncUciBestmove, ncUciInfo);
}

void ncUciBestmove(ncMove move)
{
    char uci[6];
    ncMoveUCI(move, uci);
    printf("bestmove %s\n", uci);
}

void ncUciInfo(ncSearchInfo info)
{
    int score = info.score * 2000 - 1000;

    if (info.ctm == NC_BLACK)
        score *= -1;

    printf("info depth %d score cp %d nodes %d nps %d time %d\n", info.depth, score, info.nodes, info.nps, info.elapsed);
}

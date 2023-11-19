#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define DECK_SIZE 52
#define INIT_HAND_CNT 7
#define MAX_HAND_CNT 15

typedef struct
{
    enum
    {
        SPADES = 0,
        HEARTS = 1,
        DIAMONDS = 2,
        CLUBS = 3
    } suit;

    enum
    {
        ACE = 1,
        TWO = 2,
        THREE = 3,
        FOUR = 4,
        FIVE = 5,
        SIX = 6,
        SEVEN = 7,
        EIGHT = 8,
        NINE = 9,
        JACK = 10,
        QUEEN = 11,
        KING = 12
    } rank;
} Card;

typedef struct
{
    Card cards[DECK_SIZE];
    int cur_idx;
} Deck;


typedef struct
{
    Card cards[MAX_HAND_CNT];
    int cnt;
} Hand;

void shuffle_deck(Card * deck);
bool is_valid(Card floor, Card hand);

int main()
{
    int instances;

    printf("Enter the number of instances:");
    scanf("%d", &instances);

    for (int i = 0; i < instances; i++)
    {
        
        puts("===================================");
        printf("Instance (%d/%d)\n", i + 1, instances);

        // shuffle_deck();


    }

    return 0;
}

void shuffle_deck(Card * deck)
{
    for (int i = 0; i < DECK_SIZE; i++)
    {
        deck[i].suit = i / 13;
        deck[i].rank = i % 13 + 1;
    }

    srand(time(NULL));

    for (int i = DECK_SIZE - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);

        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

bool is_valid(Card floor, Card hand)
{
    return (floor.rank == hand.rank || floor.suit == hand.suit);
}
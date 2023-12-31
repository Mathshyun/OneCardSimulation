#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define MAX_DECK_CNT 52
#define INIT_HAND_CNT 7
#define MAX_HAND_CNT 15
#define MAX_TURN_CNT 300
#define ACE_DRAWS 3
#define ACE_SPADE_DRAWS 3

typedef struct
{
    enum
    {
        NULL_SUIT = 0,
        SPADES = 1,
        HEARTS = 2,
        DIAMONDS = 3,
        CLUBS = 4
    } suit;

    enum
    {
        NULL_RANK = 0,
        ACE = 1,
        TWO = 2,
        THREE = 3,
        FOUR = 4,
        FIVE = 5,
        SIX = 6,
        SEVEN = 7,
        EIGHT = 8,
        NINE = 9,
        TEN = 10,
        JACK = 11,
        QUEEN = 12,
        KING = 13
    } rank;
} Card;

typedef struct
{
    Card cards[MAX_DECK_CNT];
    int cur_idx;
    int cnt;
} Deck;

typedef struct
{
    Card cards[MAX_HAND_CNT];
    int cnt;
} Hand;

void initialize_deck(Deck *deck);
void shuffle_floor(Deck *deck, Deck *floor);
void draw_card(Deck *deck, Deck *floor, Hand *hand, int cnt);
bool is_valid(Card floor, Card hand);
char * get_card_name(Card card);
Card play_card(Hand *hand, int idx);
Card choice_random(Hand *hand, Card floor);
Card player_choice(Hand *hand, Card floor, int opp_cnt, int method);
Card opponent_choice(Hand *hand, Card floor, int method);
Card choice_attack(Hand *hand, Card floor);
Card choice_attack_responsive(Hand *hand, Card floor, int opp_cnt, int thres);
Card choice_nonattack(Hand *hand, Card floor);
Card choice_nonattack_responsive(Hand *hand, Card floor, int opp_cnt, int thres);

FILE *fp;
bool process;
bool verbose;
int attack_threshold = 0;

int main()
{
    bool is_player_first;
    int instances;
    int wins = 0, draws = 0;
    int player_empty = 0, opponent_empty = 0;
    int player_full = 0, opponent_full = 0;
    int player_method, opponent_method;
    int x;
    char filename[32], curtime[16];

    time_t t = time(NULL);
    clock_t start;
    
    strftime(curtime, sizeof(curtime), "%Y%m%d_%H%M%S", localtime(&t));
    strcpy(filename, "log_");
    strcat(filename, curtime);
    strcat(filename, ".txt");

    fp = fopen(filename, "w");
    srand(t);

    printf("Log file: %s\n", filename);

    printf("Enter how to write log (0: result only, 1: simple, 2: detailed):");
    scanf("%d", &x);

    process = (x > 0);
    verbose = (x > 1);

    printf("Enter the number of instances:");
    scanf("%d", &instances);

    printf("Enter the player's turn (0: opponent first, 1: player first):");
    scanf("%d", &x);

    is_player_first = (x == 1);

    printf("Enter the player's method:");
    scanf("%d", &player_method);

    if (player_method == 3 || player_method == 5) // Attack responsive
    {
        printf("Enter the attack threshold:");
        scanf("%d", &attack_threshold);
    }

    printf("Enter the opponent's method:");
    scanf("%d", &opponent_method);

    if (attack_threshold) fprintf(fp, "Player method: %d, attack threshold: %d\n", player_method, attack_threshold);
    else fprintf(fp, "Player method: %d\n", player_method);
    fprintf(fp, "Opponent method: %d\n", opponent_method);
    fprintf(fp, "%s first\n", is_player_first ? "Player" : "Opponent");

    start = clock();

    for (int i = 0; i < instances; i++)
    {
        Deck deck, floor = { {}, 0, 0 };
        Hand player = { {}, 0 }, opponent = { {}, 0 };

        bool is_player_turn = is_player_first;
        int turn = 0;

        fclose(fp);
        fp = fopen(filename, "a");
        
        puts("");
        printf("===== Instance: %d/%d =====\n", i + 1, instances);

        if (process) fprintf(fp, "\n===== Instance: %d/%d =====\n", i + 1, instances);

        initialize_deck(&deck);

        draw_card(&deck, &floor, &player, INIT_HAND_CNT);
        draw_card(&deck, &floor, &opponent, INIT_HAND_CNT);

        floor.cards[floor.cnt++] = deck.cards[deck.cur_idx++];

        fclose(fp);
        fp = fopen(filename, "a");

        while (true)
        {
            turn++;

            if (turn > MAX_TURN_CNT)
            {
                puts("");
                puts("Draw");

                if (process) fputs("\nDraw\n", fp);

                draws++;
                break;
            }
            
            // Print status
            if (verbose)
            {
                fprintf(fp, "\n----- Turn: %d -----\n", turn);
                fprintf(fp, "\nDeck: %d\n", deck.cnt - deck.cur_idx);
                fputs("\nPlayer:", fp);
                for (int i = 0; i < player.cnt; i++)
                {
                    fprintf(fp, " %s", get_card_name(player.cards[i]));
                }
                fputs("\nOpponent:", fp);
                for (int i = 0; i < opponent.cnt; i++)
                {
                    fprintf(fp, " %s", get_card_name(opponent.cards[i]));
                }
                fprintf(fp, "\nFloor: %s\n", get_card_name(floor.cards[floor.cnt - 1]));
                fprintf(fp, "%s's turn\n", is_player_turn ? "Player" : "Opponent");
            }
            
            if (is_player_turn)
            {
                Card card = player_choice(&player, floor.cards[floor.cnt - 1], opponent.cnt, player_method);

                if (card.suit == NULL_SUIT)
                {
                    draw_card(&deck, &floor, &player, 1);

                    if (verbose) fputs("\nPlayer draws a card\n", fp);

                    if (player.cnt >= MAX_HAND_CNT)
                    {
                        puts("");
                        puts("Player's hand is full, Player loses");

                        if (process) fputs("\nPlayer's hand is full\nPlayer loses\n", fp);

                        player_full++;
                        break;
                    }
                    
                    is_player_turn = false;
                }
                else
                {
                    bool hand_full = false;
                    
                    floor.cards[floor.cnt++] = card;

                    if (verbose) fprintf(fp, "\nPlayer plays %s\n", get_card_name(card));

                    if (player.cnt == 0)
                    {
                        puts("");
                        puts("Player's hand is empty, Player wins");

                        if (process) fputs("\nPlayer's hand is empty\nPlayer wins\n", fp);

                        wins++;
                        player_empty++;
                        break;
                    }

                    switch (floor.cards[floor.cnt - 1].rank)
                    {
                        case ACE:
                            draw_card(&deck, &floor, &opponent, (floor.cards[floor.cnt - 1].suit == SPADES) ? ACE_SPADE_DRAWS : ACE_DRAWS);

                            if (verbose) fprintf(fp, "\nOpponent draws %d cards\n", (floor.cards[floor.cnt - 1].suit == SPADES) ? ACE_SPADE_DRAWS : ACE_DRAWS);

                            if (opponent.cnt >= MAX_HAND_CNT)
                            {
                                puts("");
                                puts("Opponent's hand is full, Player wins");

                                if (process) fputs("\nOpponent's hand is full\nPlayer wins\n", fp);

                                hand_full = true;
                                wins++;
                                opponent_full++;
                                break;
                            }

                            is_player_turn = false;
                            break;
                        case TWO:
                            draw_card(&deck, &floor, &opponent, 2);

                            if (verbose) fputs("\nOpponent draws 2 cards\n", fp);

                            if (opponent.cnt >= MAX_HAND_CNT)
                            {
                                puts("");
                                puts("Opponent's hand is full, Player wins");

                                if (process) fputs("\nOpponent's hand is full\nPlayer wins\n", fp);

                                hand_full = true;
                                wins++;
                                opponent_full++;
                                break;
                            }

                            is_player_turn = false;
                            break;
                        case JACK:
                        case QUEEN:
                        case KING:
                            // Don't change turn
                            break;
                        default:
                            is_player_turn = false;
                            break;
                    }

                    if (hand_full) break;
                }
            }
            else    // Opponent's turn
            {
                Card card = opponent_choice(&opponent, floor.cards[floor.cnt - 1], opponent_method);

                if (card.suit == NULL_SUIT)
                {
                    draw_card(&deck, &floor, &opponent, 1);
                    if (verbose) fputs("\nOpponent draws a card\n", fp);

                    if (opponent.cnt >= MAX_HAND_CNT)
                    {
                        puts("");
                        puts("Opponent's hand is full, Player wins");

                        if (process) fputs("\nOpponent's hand is full\nPlayer wins\n", fp);

                        wins++;
                        opponent_full++;
                        break;
                    }

                    is_player_turn = true;
                }
                else
                {
                    bool hand_full = false;
                    
                    floor.cards[floor.cnt++] = card;

                    if (verbose) fprintf(fp, "\nOpponent plays %s\n", get_card_name(card));

                    if (opponent.cnt == 0)
                    {
                        puts("");
                        puts("Opponent's hand is empty, Player loses");

                        if (process) fputs("\nOpponent's hand is empty\nPlayer loses\n", fp);

                        opponent_empty++;
                        break;
                    }

                    switch (floor.cards[floor.cnt - 1].rank)
                    {
                        case ACE:
                            draw_card(&deck, &floor, &player, (floor.cards[floor.cnt - 1].suit == SPADES) ? ACE_SPADE_DRAWS : ACE_DRAWS);

                            if (verbose) fprintf(fp, "\nPlayer draws %d cards\n", (floor.cards[floor.cnt - 1].suit == SPADES) ? ACE_SPADE_DRAWS : ACE_DRAWS);

                            if (player.cnt >= MAX_HAND_CNT)
                            {
                                puts("");
                                puts("Player's hand is full, Player loses");

                                if (process) fputs("\nPlayer's hand is full\nPlayer loses\n", fp);

                                hand_full = true;
                                player_full++;
                                break;
                            }

                            is_player_turn = true;
                            break;
                        case TWO:
                            draw_card(&deck, &floor, &player, 2);

                            if (verbose) fputs("\nPlayer draws 2 cards\n", fp);

                            if (player.cnt >= MAX_HAND_CNT)
                            {
                                puts("");
                                puts("Player's hand is full, Player loses");

                                if (process) fputs("\nPlayer's hand is full\nPlayer loses\n", fp);

                                hand_full = true;
                                player_full++;
                                break;
                            }

                            is_player_turn = true;
                            break;
                        case JACK:
                        case QUEEN:
                        case KING:
                            // Don't change turn
                            break;
                        default:
                            is_player_turn = true;
                            break;
                    }

                    if (hand_full) break;
                }
            }
        }
        fclose(fp);
        fp = fopen(filename, "a");
    }
    
    puts("");
    puts("==================================================");
    printf("%.3lf second(s) elapsed\n", (float) (clock() - start) / 1000000);
    puts("");
    printf("Win:                           %10d, %6.2f%%\n", wins, (float) wins / instances * 100);
    printf("  by emptying player's hand:   %10d, %6.2f%%\n", player_empty, (float) player_empty / instances * 100);
    printf("  by filling opponent's hand:  %10d, %6.2f%%\n", opponent_full, (float) opponent_full / instances * 100);
    puts("");
    printf("Lose:                          %10d, %6.2f%%\n", instances - wins - draws, (float) (instances - wins - draws) / instances * 100);
    printf("  by emptying opponent's hand: %10d, %6.2f%%\n", opponent_empty, (float) opponent_empty / instances * 100);
    printf("  by filling player's hand:    %10d, %6.2f%%\n", player_full, (float) player_full / instances * 100);
    puts("");
    printf("Draw:                          %10d, %6.2f%%\n", draws, (float) draws / instances * 100);
    puts("");
    puts("--------------------------------------------------");
    printf("Total:                         %10d, 100.00%%\n", instances);


    fputs("\n==================================================\n\n", fp);
    fprintf(fp, "Win:                           %10d, %6.2f%%\n", wins, (float) wins / instances * 100);
    fprintf(fp, "  by emptying player's hand:   %10d, %6.2f%%\n", player_empty, (float) player_empty / instances * 100);
    fprintf(fp, "  by filling opponent's hand:  %10d, %6.2f%%\n", opponent_full, (float) opponent_full / instances * 100);
    fputs("\n", fp);
    fprintf(fp, "Lose:                          %10d, %6.2f%%\n", instances - wins - draws, (float) (instances - wins - draws) / instances * 100);
    fprintf(fp, "  by emptying opponent's hand: %10d, %6.2f%%\n", opponent_empty, (float) opponent_empty / instances * 100);
    fprintf(fp, "  by filling player's hand:    %10d, %6.2f%%\n", player_full, (float) player_full / instances * 100);
    fputs("\n", fp);
    fprintf(fp, "Draw:                          %10d, %6.2f%%\n", draws, (float) draws / instances * 100);
    fputs("\n--------------------------------------------------\n", fp);
    fprintf(fp, "Total:                         %10d, 100.00%%\n", instances);

    fclose(fp);
    return 0;
}

void initialize_deck(Deck *deck)
{
    deck->cur_idx = 0;
    deck->cnt = MAX_DECK_CNT;
    
    for (int i = 0; i < MAX_DECK_CNT; i++)
    {
        deck->cards[i].suit = i / 13 + 1;
        deck->cards[i].rank = i % 13 + 1;
    }

    for (int i = MAX_DECK_CNT - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);

        Card temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }
}

void shuffle_floor(Deck *deck, Deck *floor)
{
    deck->cur_idx = 0;
    deck->cnt = floor->cnt - 1;

    for (int i = 0; i < deck->cnt; i++)
    {
        deck->cards[i] = floor->cards[i];
    }

    for (int i = deck->cnt - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);

        Card temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }

    floor->cards[0] = floor->cards[floor->cnt - 1];

    floor->cur_idx = 0;
    floor->cnt = 1;

    if (verbose) fputs("\nFloor is shuffled\n", fp);
}

void draw_card(Deck *deck, Deck *floor, Hand *hand, int cnt)
{
    for (int i = 0; i < cnt; i++)
    {
        if (deck->cur_idx == deck->cnt) shuffle_floor(deck, floor);
        
        hand->cards[hand->cnt++] = deck->cards[deck->cur_idx++];
    }
}

bool is_valid(Card floor, Card hand)
{
    return (floor.rank == hand.rank || floor.suit == hand.suit);
}

char * get_card_name(Card card)
{
    static char name[2];

    switch (card.suit)
    {
        case SPADES:
            name[0] = 'S';
            break;
        case HEARTS:
            name[0] = 'H';
            break;
        case DIAMONDS:
            name[0] = 'D';
            break;
        case CLUBS:
            name[0] = 'C';
            break;
        default:
            name[0] = '*';
            break;
    }

    switch (card.rank)
    {
        case ACE:
            name[1] = 'A';
            break;
        case TEN:
            name[1] = 'X';
            break;
        case JACK:
            name[1] = 'J';
            break;
        case QUEEN:
            name[1] = 'Q';
            break;
        case KING:
            name[1] = 'K';
            break;
        default:
            name[1] = card.rank + '0';
            break;
    }

    return name;
}

Card play_card(Hand *hand, int idx)
{
    Card temp = hand->cards[idx];

    for (int i = idx + 1; i < hand->cnt; i++)
    {
        hand->cards[i - 1] = hand->cards[i];
    }
    hand->cnt--;

    return temp;
}

Card choice_random(Hand *hand, Card floor)
{
    int valid_idx[hand->cnt];
    int valid_cnt = 0;
    int idx;

    for (int i = 0; i < hand->cnt; i++)
    {
        if (is_valid(floor, hand->cards[i]))
        {
            valid_idx[valid_cnt++] = i;
        }
    }

    if (valid_cnt == 0) return (Card) { NULL_SUIT, NULL_RANK };

    idx = valid_idx[rand() % valid_cnt];

    return play_card(hand, idx);
}

Card player_choice(Hand *hand, Card floor, int opp_cnt, int method)
{
    switch (method)
    {
        case 1:
            // Choose the first valid card
            for (int i = 0; i < hand->cnt; i++)
            {
                if (is_valid(floor, hand->cards[i]))
                {
                    return play_card(hand, i);
                }
            }

            return (Card) { NULL_SUIT, NULL_RANK };

        case 2:
            // Attack card first
            return choice_attack(hand, floor);
        
        case 3:
            // Attack card first, responsive
            return choice_attack_responsive(hand, floor, opp_cnt, attack_threshold);

        case 4:
            // Non-attack card first
            return choice_nonattack(hand, floor);
        
        case 5:
            // Non-attack card first, responsive
            return choice_nonattack_responsive(hand, floor, opp_cnt, attack_threshold);
        
        default:
            // choose random valid card
            return choice_random(hand, floor);
    }
}

Card opponent_choice(Hand *hand, Card floor, int method)
{
    switch (method)
    {
        default:
            return choice_random(hand, floor);
    }
}

Card choice_attack(Hand *hand, Card floor)
{
    int valid_idx[hand->cnt];
    int valid_cnt = 0;
    int idx;

    // Search for attack cards
    for (int i = 0; i < hand->cnt; i++)
    {
        if (is_valid(floor, hand->cards[i]))
        {
            if (hand->cards[i].rank == ACE || hand->cards[i].rank == TWO)
            {
                valid_idx[valid_cnt++] = i;
            }
        }
    }

    // If there is no attack card, search for other cards
    if (valid_cnt == 0)
    {
        for (int i = 0; i < hand->cnt; i++)
        {
            if (is_valid(floor, hand->cards[i]))
            {
                valid_idx[valid_cnt++] = i;
            }
        }
    }

    // If there is no valid card, return NULL card
    if (valid_cnt == 0) return (Card) { NULL_SUIT, NULL_RANK };

    idx = valid_idx[rand() % valid_cnt];

    return play_card(hand, idx);
}

Card choice_attack_responsive(Hand *hand, Card floor, int opp_cnt, int thres)
{
    int valid_idx[hand->cnt];
    int valid_cnt = 0;
    int idx;

    // Search for attack cards
    if (opp_cnt <= thres)
    {
        for (int i = 0; i < hand->cnt; i++)
        {
            if (is_valid(floor, hand->cards[i]))
            {
                if (hand->cards[i].rank == ACE || hand->cards[i].rank == TWO)
                {
                    valid_idx[valid_cnt++] = i;
                }
            }
        }
    }

    if (valid_cnt == 0)
    {
        for (int i = 0; i < hand->cnt; i++)
        {
            if (is_valid(floor, hand->cards[i]))
            {
                valid_idx[valid_cnt++] = i;
            }
        }
    }

    if (valid_cnt == 0) return (Card) { NULL_SUIT, NULL_RANK };

    idx = valid_idx[rand() % valid_cnt];

    return play_card(hand, idx);
}

Card choice_nonattack(Hand *hand, Card floor)
{
    int valid_idx[hand->cnt];
    int valid_cnt = 0;
    int idx;

    // Search for non-attack cards
    for (int i = 0; i < hand->cnt; i++)
    {
        if (is_valid(floor, hand->cards[i]))
        {
            if (hand->cards[i].rank != ACE && hand->cards[i].rank != TWO)
            {
                valid_idx[valid_cnt++] = i;
            }
        }
    }

    // If there is no non-attack card, search for other cards
    if (valid_cnt == 0)
    {
        for (int i = 0; i < hand->cnt; i++)
        {
            if (is_valid(floor, hand->cards[i]))
            {
                valid_idx[valid_cnt++] = i;
            }
        }
    }

    // If there is no valid card, return NULL card
    if (valid_cnt == 0) return (Card) { NULL_SUIT, NULL_RANK };

    idx = valid_idx[rand() % valid_cnt];

    return play_card(hand, idx);
}

Card choice_nonattack_responsive(Hand *hand, Card floor, int opp_cnt, int thres)
{
    int valid_idx[hand->cnt];
    int valid_cnt = 0;
    int idx;

    // Search for attack cards
    if (opp_cnt <= thres)
    {
        for (int i = 0; i < hand->cnt; i++)
        {
            if (is_valid(floor, hand->cards[i]))
            {
                if (hand->cards[i].rank == ACE || hand->cards[i].rank == TWO)
                {
                    valid_idx[valid_cnt++] = i;
                }
            }
        }
    }

    // Search for non-attack cards
    else
    {
        if (opp_cnt > thres)
        {
            for (int i = 0; i < hand->cnt; i++)
            {
                if (is_valid(floor, hand->cards[i]))
                {
                    if (hand->cards[i].rank != ACE && hand->cards[i].rank != TWO)
                    {
                        valid_idx[valid_cnt++] = i;
                    }
                }
            }
        }
    }

    // Search for other cards
    if (valid_cnt == 0)
    {
        for (int i = 0; i < hand->cnt; i++)
        {
            if (is_valid(floor, hand->cards[i]))
            {
                valid_idx[valid_cnt++] = i;
            }
        }
    }

    if (valid_cnt == 0) return (Card) { NULL_SUIT, NULL_RANK };

    idx = valid_idx[rand() % valid_cnt];

    return play_card(hand, idx);
}
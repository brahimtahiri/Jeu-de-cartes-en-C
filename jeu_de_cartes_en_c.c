/**
 * =====================================================================
 * Jeu de cartes en C
 * 
 * Réalisé par : Brahim TAHIRI et Ian Aouchiche
 * Projet disponible sur GitHub : https://github.com/brahimtahiri/Jeu-de-cartes-en-C
 * =====================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>

#define DECK_SIZE 32
#define HAND_SIZE 16

typedef struct {
    int value;
    char suit;
} Card;

Card pick_card(Card hand[], int *nb, char demanded_suit, int lead);

/**
 * Mélange les cartes d’un paquet en utilisant l’algorithme de Fisher-Yates.
 *
 * @param deck  Tableau de cartes représentant le paquet à mélanger.
 *
 * Cet algorithme garantit un mélange uniforme en permutant chaque carte avec
 * une autre carte choisie aléatoirement parmi celles qui la précèdent (ou elle-même).
 */
void shuffle(Card *deck) {
    for (int i = DECK_SIZE - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        Card tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
}

/**
 * Affiche une carte au format lisible.
 *
 * @param c  Carte à afficher. La valeur est convertie en chaîne (7, 8, 9, ..., V, D, R, A)
 *           et suivie de sa couleur (ex. : 'H' pour cœur, 'S' pour pique, etc.).
 *
 * Exemple de sortie : "10H", "VD", "AS"
 */
void print_card(Card c) {
    const char *names[] = {"","", "", "", "", "", "", "7","8","9","10","V","D","R","A"};
    printf("%s%c", names[c.value], c.suit);
}

/**
 * Sélectionne une carte à jouer depuis la main du joueur.
 *
 * @param hand           Tableau de cartes représentant la main du joueur.
 * @param nb             Pointeur vers le nombre de cartes dans la main.
 * @param demanded_suit  Couleur demandée pour le pli en cours ('H', 'S', 'D', 'C', etc.).
 * @param lead           Booléen indiquant si le joueur commence le pli (1 si oui, 0 sinon).
 *
 * @return La carte choisie selon les règles suivantes :
 *  - Si le joueur mène (lead == 1), il joue la première carte de sa main.
 *  - Sinon, il essaie de jouer la carte de plus haute valeur correspondant à la couleur demandée.
 *  - Si aucune carte ne correspond à la couleur demandée, il joue la dernière carte de sa main.
 * La carte jouée est retirée de la main (le tableau est mis à jour et le nombre de cartes décrémenté).
 */
Card pick_card(Card hand[], int *nb, char demanded_suit, int lead) {
    int idx = -1;
    if (lead) {
        idx = 0;
    } else {
        int best = -1;
        for (int i = 0; i < *nb; ++i) {
            if (hand[i].suit == demanded_suit) {
                if (best == -1 || hand[i].value > hand[best].value)
                    best = i;
            }
        }
        idx = (best != -1) ? best : *nb - 1;
    }
    Card chosen = hand[idx];
    for (int i = idx; i < *nb - 1; ++i) hand[i] = hand[i + 1];
    (*nb)--;
    return chosen;
}

/**
 * Fonction principale du programme : réalise une partie de cartes entre deux processus joueurs.
 *
 * - Initialise un paquet de cartes, le mélange et le distribue aux deux joueurs.
 * - Crée deux processus enfants via `fork()`, chacun jouant le rôle d’un joueur.
 * - Met en place la communication entre le parent (arbitre) et les enfants (joueurs) à l’aide de pipes.
 * - Gère chaque tour de jeu :
 *     - Le joueur en tête joue une carte (commande 'L'), puis l’autre suit (commande 'F').
 *     - Le parent compare les cartes pour déterminer le gagnant du pli.
 *     - Le score est mis à jour, et le leader du tour suivant est désigné.
 * - À la fin de la partie, les processus enfants sont informés de la fin du jeu (commande 'X').
 * - Affiche le score final et déclare le gagnant.
 *
 * @return 0 si le programme se termine correctement.
 */
int main() {
    srand(time(NULL));
    Card deck[DECK_SIZE];
    char suits[4] = {'C','D','H','S'};
    int pos = 0;
    for (int s = 0; s < 4; ++s)
        for (int v = 7; v <= 14; ++v)
            deck[pos++] = (Card){v, suits[s]};
    shuffle(deck);

    int toChild1[2], toChild2[2];
    int fromChild1[2], fromChild2[2];
    pipe(toChild1); pipe(toChild2);
    pipe(fromChild1); pipe(fromChild2);

    pid_t p1 = fork();
    if (p1 == 0) {
        close(toChild1[1]); close(fromChild1[0]);
        Card hand[HAND_SIZE];
        read(toChild1[0], hand, sizeof(hand));
        int nb = HAND_SIZE;
        char cmd;
        Card card_buffer;
        while (read(toChild1[0], &cmd, 1) > 0) {
            if (cmd == 'L') {
                Card played = pick_card(hand, &nb, 0, 1);
                write(fromChild1[1], &played, sizeof(played));
            } else if (cmd == 'F') {
                read(toChild1[0], &card_buffer, sizeof(card_buffer));
                Card played = pick_card(hand, &nb, card_buffer.suit, 0);
                write(fromChild1[1], &played, sizeof(played));
            } else if (cmd == 'X') break;
        }
        close(toChild1[0]); close(fromChild1[1]);
        return 0;
    }

    pid_t p2 = fork();
    if (p2 == 0) {
        close(toChild2[1]); close(fromChild2[0]);
        Card hand[HAND_SIZE];
        read(toChild2[0], hand, sizeof(hand));
        int nb = HAND_SIZE;
        char cmd;
        Card card_buffer;
        while (read(toChild2[0], &cmd, 1) > 0) {
            if (cmd == 'L') {
                Card played = pick_card(hand, &nb, 0, 1);
                write(fromChild2[1], &played, sizeof(played));
            } else if (cmd == 'F') {
                read(toChild2[0], &card_buffer, sizeof(card_buffer));
                Card played = pick_card(hand, &nb, card_buffer.suit, 0);
                write(fromChild2[1], &played, sizeof(played));
            } else if (cmd == 'X') break;
        }
        close(toChild2[0]); close(fromChild2[1]);
        return 0;
    }

    close(toChild1[0]); close(toChild2[0]);
    close(fromChild1[1]); close(fromChild2[1]);

    write(toChild1[1], deck, sizeof(Card) * HAND_SIZE);
    write(toChild2[1], deck + HAND_SIZE, sizeof(Card) * HAND_SIZE);

    int score1 = 0, score2 = 0;
    int leader = 1;
    for (int round = 0; round < HAND_SIZE; ++round) {
        char leadCmd = 'L', followCmd = 'F';
        if (leader == 1) {
            write(toChild1[1], &leadCmd, 1);
            write(toChild2[1], &followCmd, 1);
        } else {
            write(toChild2[1], &leadCmd, 1);
            write(toChild1[1], &followCmd, 1);
        }

        Card cardLead, cardFollow;
        if (leader == 1) {
            read(fromChild1[0], &cardLead, sizeof(cardLead));
            write(toChild2[1], &cardLead, sizeof(cardLead));
            read(fromChild2[0], &cardFollow, sizeof(cardFollow));
        } else {
            read(fromChild2[0], &cardLead, sizeof(cardLead));
            write(toChild1[1], &cardLead, sizeof(cardLead));
            read(fromChild1[0], &cardFollow, sizeof(cardFollow));
        }

        int winner = leader;
        if (cardFollow.suit == cardLead.suit) {
            if (cardFollow.value > cardLead.value)
                winner = (leader == 1) ? 2 : 1;
        }

        if (winner == 1) score1++; else score2++;
        leader = winner;

        printf("Pli %d : J1 ", round + 1); print_card(leader == 1 ? cardLead : cardFollow);
        printf("  vs  J2 "); print_card(leader == 1 ? cardFollow : cardLead);
        printf("  -> Gagnant : J%d\n", winner);
    }

    char endCmd = 'X';
    write(toChild1[1], &endCmd, 1);
    write(toChild2[1], &endCmd, 1);
    close(toChild1[1]); close(toChild2[1]);
    close(fromChild1[0]); close(fromChild2[0]);

    wait(NULL); wait(NULL);

    printf("\nScore final : Joueur1 = %d, Joueur2 = %d\n", score1, score2);
    if (score1 > score2) printf("Joueur1 gagne !\n");
    else if (score2 > score1) printf("Joueur2 gagne !\n");
    else printf("Egalité !\n");
    return 0;
}

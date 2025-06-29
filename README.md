# Jeu de cartes en C

Ce projet est un TP de programmation en C qui simule un jeu de cartes entre deux joueurs, implémenté avec des processus et une communication via pipes sous Linux.

## Description

* Le programme initialise un paquet de 32 cartes (valeurs 7 à As, 4 couleurs).
* Les cartes sont mélangées avec l’algorithme de Fisher-Yates.
* Le paquet est distribué entre deux processus enfants représentant les joueurs.
* La partie se joue en plusieurs tours où les joueurs jouent chacun une carte.
* Le parent arbitre la partie, compare les cartes, détermine le gagnant de chaque pli, et affiche le score final.

## Fonctionnalités

* Utilisation des processus (`fork`) pour simuler les joueurs.
* Communication inter-processus avec des `pipes`.
* Gestion des règles simples du jeu de cartes.
* Affichage des plis et du score en temps réel.

## Compilation

Assurez-vous d’avoir un compilateur C installé (gcc).

```bash
gcc -o jeu_de_cartes jeu_de_cartes.c
```

## Exécution

```bash
./jeu_de_cartes
```

Le programme affiche le déroulement de la partie et le score final.

## Auteurs

* Brahim TAHIRI
* Ian Aouchiche

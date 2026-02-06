# Algorithmos : Plateforme de Visualisation de Structures de Données

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![C](https://img.shields.io/badge/language-C-orange.svg)
![GTK+](https://img.shields.io/badge/UI-GTK%2B3-green.svg)

Cette application desktop permet l'exploration, l'animation et l'analyse de structures de données fondamentales ainsi que des algorithmes de tri et de recherche associés.

---

## Fonctionnalités Principales

### Visualisation Interactive
*   **Tableaux** : Visualisation en temps réel des algorithmes de tri (Bulle, Sélection, Shell, Quick Sort) avec suivi du pseudo-code.
*   **Listes Chaînées** : Gestion de listes simples et doubles avec animations pour les opérations d'insertion et de suppression.
*   **Arbres** : Représentation d'arbres binaires et N-aires incluant les différents modes de parcours (BFS, DFS).
*   **Graphes** : Création de réseaux, gestion de l'orientation et des pondérations, et implémentation d'algorithmes de recherche de chemin (Dijkstra, Bellman-Ford, Floyd-Warshall).

### Analyse de Performance
*   **Benchmarks** : Analyse comparative de l'efficacité algorithmique basée sur des volumes de données variables.
*   **Export PNG** : Possibilité d'exporter les graphiques d'analyse en haute résolution pour une intégration documentaire.

### Interface et Expérience Utilisateur
*   **Design Moderne** : Interface soignée utilisant des concepts de design contemporains.
*   **Thèmes Personnalisables** : Support complet des modes sombre et clair.
*   **Journal de Session** : Suivi détaillé de l'historique des opérations effectuées.

---

## Installation et Utilisation

### Prérequis
*   Un compilateur C (type GCC).
*   Bibliothèque de développement GTK+ 3.0.

### Compilation
Exemple de commande de compilation (environnement Linux ou Windows MSYS2) :
```bash
gcc main.c -o DataStructureProject `pkg-config --cflags --libs gtk+-3.0`
```

---

## Structure du Projet
*   `main.c` : Code source principal regroupant la logique algorithmique et l'interface utilisateur.
*   `.gitignore` : Définition des fichiers à exclure du suivi de version.
*   `README.md` : Documentation technique du projet.

---

## Technologies Utilisées
*   **Langage** : C
*   **Interface Graphique** : GTK+ 3
*   **Rendu Vectoriel** : Cairo Graphics
*   **Mise en forme** : CSS personnalisé

---

## Informations sur le Projet
*   **Cadre** : Projet académique - Structures de Données et Programmation C.
*   **Date de publication** : Février 2026.

---
*Ce projet a été conçu dans un but pédagogique pour faciliter la compréhension des structures de données complexes par la visualisation.*

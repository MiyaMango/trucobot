## TrucoBot
This project was initially developed in my first semester as a student in the University of São Paulo for an electronics class. It has since grown in scope, and development is still ongoing. The current goal is to develop this as a hands-on project that can be reproduced in classrooms or by interested individuals for educational purposes.

The goal of this project is to construct a physical robot that can play a game of [Truco](https://en.wikipedia.org/wiki/Truco), a card game popular in Brazil. For simplicity, the two-version player of the game was chosen, and the "Truco mineiro" variety was chosen for implementation. 

<img width="1524" height="845" alt="trucobotitself" src="https://github.com/user-attachments/assets/2b110f50-1c80-407e-9d7b-6ee0d5168cc1" />


## Implementation
*This project was made when i was first learning to program, and as such is very dirty mixture of C and C++. I will be making a cleaner version soon, and a historical version will be archived.*

The project implements a basic finite state machine to handle the game logic. The game state is stored in a series of global variables. Each playing card has a unique ID stored in its NFC sticker tag, which allows the robot to interact physically with the player through its servo-motors.  

When it is the player's turn, the game will wait for an input from the player, which can be playing a card (by physically approaching one of the NFC tags to the NFC reader), or pressing one of the 3 push buttons, each corresponding to an in-game action (truco/raise or fold). Invalid actions will be ignored.

When it is the robot's turn, the robot will make an appropriate choice for the current game state, being able to play one of its own cards (by physically flipping over the card placed on top of the servo), or communicating the intent to raise or fold to the player through the LCD screen. 

The robot uses a simple AI to make game choices. It uses a simplified [Monte Carlo simulation](https://en.wikipedia.org/wiki/Monte_Carlo_method) to tell how strong its hand is, assigning a "confidence value" between 0 and 100 to its starting hand. By grouping the cards by equal strength, we can reduce the possible enemy hands to only around 400 distinct arrangements after the robot has already received it's own cards. This low number makes it possible to simply simulate a round of the game played by every hand that can exist playing against every possible adversary hand, using a simple algorithm (near-ideal play strategy, assumes no bluffing).

Because the Arduino UNO only has 2kb of RAM (and about three-quarters is already used by this program and the imported libraries), it cannot run this simulation during play. Therefore, this simulation was pre-computed, and its results were inserted in the 'confiancas' table, stored in program memory. The "confidence" statistic is then a measurement of how well it did during this test: 0 means it wins against 0% of possible adversary hands, and 100 means it won against 100% of possible adversary hands. To obtain the confidence value of a given hand, the program looks up its hand in the table and fetches the corresponding confidence value.

Bluffing and unpredictability are essential strategies in a game of Truco. To emulate this, the robot uses a random factor along with its confidence factor in decision-making, leading to a variable but generally high-quality play strategy, ocassionally bluffing when it has weak cards. The flowchart below represents the play logic used:

**todo: decision flowchart**

Libraries used:

[Miguel Balboa's RFID Library](https://github.com/miguelbalboa/rfid)

[LCD_I2C](https://github.com/blackhack/LCD_I2C) 

[Vector for Arduino](https://github.com/janelia-arduino/Vector)

Other libraries used are Arduino libraries.

## Physical Construction 
If you want to recreate this project yourself, here is a list of components used:
| Component        | # used |
| ---              | - |
| Arduino UNO      | 1 |
| SG90 Servo       | 3 |
| Passive Buzzer   | 1 |
| 16x2 LCD Display | 1 |
| I2C Adapter      | 1 |
| Push Button      | 3 |
| RC522 NFC Reader | 1 |
| 5V Power supply  | 1 |
| Mifare Ultralight C stickers | 40 |

You'll also need a shoebox or similar for the body, something to put the NFC stickers on (preferably a deck of cards), and basic craft materials, such as hot glue and a box cutter. The initial version of this project was built using a breadboard and standard jumpers, and while this is possible, this construction is very fragile, and the arduino will shut down and lose the game state if you bump on the box too hard. I am currently working on making a version built on a home-printed PCB to solve this problem.

The circuit diagram is as follows:

<img width="1265" height="871" alt="Schematic_truco_2026-08-05(1)" src="https://github.com/user-attachments/assets/45953e1b-8108-4d8b-9ba6-06996dfa02fa" />

## TO-DO LIST
The current goal of this project is to improve on the initial version, and make the construction of the robot a reproducible activity for educational purposes. As such, the planned features revolve around cleaning up existing code, and providing tutorials and tools for reproducing the project.

Planned features:
- ESP32 version, including circuit diagram + PCB design
- Program for automatically assigning ID's to every NFC
- Full code rewrite
- Video walkthrough for construction of the robot


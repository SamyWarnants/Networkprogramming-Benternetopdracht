# Networkprogramming Benternetassignment Tamagotchiland Project


![Overview](./Resources/tamagotchi.png)

## About the project

What I hope to achieve with this project is to make a **Tamagotchiland** that works on the Benternet. When the user logs in they can name their pet and the game will start. Not all features will currently work of the Tamagotchi since each one can differs a lot in functionality. That is why the focus will be on the most basic features. Hunger levels, happiness levels, hygiene levels and a minigame. After some time has passed and the Tamagotchi is not fed, cleaned or happy enough it will remind the user 3 times. When that hasn't happend the animal will say goodbye and **die**. To entertain the Tamagotchi the user will be able to play a numbergame this will raise their happiness level since they love guessing :).

**Sequence of operation**:

1. The client subscribes to ``Tamagotchiland>CreatePet!>Login``.

2. There the service will publish the following for now the name of the pet will be ``Tamagotchiland>CreatePet?>Larry``.

3. To confirm that the pet is created the user will see the following:

   1. When it happens correctly the service will send: 
      1. ``Hi thank you for creating me! To play with me go to Tamagotchiland>PetPark!>Larry ``

   2. When it happens incorrectly the service will send:
      1. ``Oh no it seems that the dark magic hasn't worked please try again``
      
         

4. Now the client will subscribe and be brought to the **PetPark** here the client will be able to interact with their pet. These are the options:

   

   1. Stats: By using ``Tamagotchiland>PetPark!>Larry>Stats`` the client will be able to see their pets stats like this:

      1. ``Tamagotchiland>PetPark?>Larry>Stats>Happiness>100%``

      2. ``Tamagotchiland>PetPark?>Larry>Stats>Hunger>100%``

      3. ``Tamagotchiland>PetPark?>Larry>Stats>Hygiene>100%``

         

   2. Play: by using ``Tamagotchiland>PetPark!>Larry>Play`` a number guessing game will start like this:

      

      1. ``Tamagotchiland>PetPark?>Larry>Play> Okay I have a number start guessing``

         1. To which the client will start guessing between 0 and 10: the player can guess like this:

            1. ``Tamagotchiland>PetPark!>Larry>Play>1``

         2. The pet will respond accordingly with either "Y" or "N" until the number is guessed like this:

            1. ``Tamagotchiland>PetPark?>Larry>Play>n``
         
            
         

   3. The option is Petcare which can be accessed like this:

      

      1. The user must type in``Tamagotchiland>PetPark!>Larry>Petcare``

         1. To feed the pet the client sends ``Tamagotchiland>PetPark!>Larry>Petcare>Feeding``
         2. To clean the pet the client sends ``Tamagotchiland>PetPark!>Larry>Petcare>Cleaning``

         

      2. The pet will then respond with ``Tamagotchiland>PetPark?>Larry>Petcare>Thank you for cleaning me!`` or ``Tamagotchiland>PetPark?>Larry>Petcare> "Thank you for Feeding me!"``.

         

   4. When the pet is not kept properly it will say "HEY ATTENTION PLEASE" which will repeat 3 times with a delay of 10 minutes between each calls until the Pet passes away. Trying to access the pet will not work and say The pet has died.

   5. Finally there is the option for logs ``Tamagotchiland>PetPark!>Larry>Logs``. By doing this it will give the user a log of all interactions.

      


## Logic flowchart **to be updated moet van netwerklogica zijn**

```mermaid
flowchart TD
    Start --> Login[Subscribe to CreatePet!Login]
    Login --> CreatePet[Service publishes CreatePet?>Larry]
    CreatePet --> CheckCreation{Pet Created?}

    CheckCreation -->|Yes| Success[Hi thank you for creating me! Go to PetPark!Larry]
    CheckCreation -->|No| Fail[Dark magic failed. Please try again]

    Success --> PetPark[Subscribe to PetPark!Larry]

    PetPark --> StatsOption[Option: Stats]
    StatsOption --> StatsRequest[Client sends PetPark!Larry>Stats]
    StatsRequest --> Happiness[Happiness > 100%]
    StatsRequest --> Hunger[Hunger > 100%]
    StatsRequest --> Hygiene[Hygiene > 100%]

    PetPark --> PlayOption[Option: Play]
    PlayOption --> PlayRequest[Client sends PetPark!Larry>Play]
    PlayRequest --> PetPrompt[Okay I have a number, start guessing]
    PetPrompt --> GuessLoop{Number guessed?}
    GuessLoop -->|No| Retry[Client sends Play>1, Pet responds Play>n]
    Retry --> GuessLoop
    GuessLoop -->|Yes| Win[Pet: Yay! You guessed it!]

    PetPark --> PetcareOption[Option: Petcare]
    PetcareOption --> PetcareOpen[Client sends Petcare]
    PetcareOpen --> Feed[Client sends Petcare>Feeding]
    PetcareOpen --> Clean[Client sends Petcare>Cleaning]
    Feed --> FeedReply[Pet: Thank you for Feeding me!]
    Clean --> CleanReply[Pet: Thank you for Cleaning me!]

    PetPark --> LogsOption[Option: Logs]
    LogsOption --> LogsRequest[Client sends PetPark!Larry>Logs]
    LogsRequest --> LogOutput[Returns interaction logs]

    PetPark --> Neglect[Pet is neglected]
    Neglect --> Warnings[HEY ATTENTION PLEASE x3]
    Warnings --> PetDies[Pet has died]
    PetDies --> DeadAccess[Accessing pet shows: Pet has died]
```

## Communicationchart

```mermaid
sequenceDiagram
    participant Client
    participant Benternet

    %% CreatePet flow
    Client->>Benternet: SUB Tamagotchiland>CreatePet!>Login
    Benternet-->>Client: PUB Tamagotchiland>CreatePet?>Larry

    alt ✅ Pet creation successfull
        Benternet-->>Client: Hi thank you for creating me! Go to Tamagotchiland>PetPark!>Larry
    else ❌ Creation failed
        Benternet-->>Client: Oh no, dark magic failed. Please try again
    end

    %% Enter PetPark
    Client->>Benternet: SUB Tamagotchiland>PetPark!>Larry

    %% Stats
    Client->>Benternet: PUB Tamagotchiland>PetPark!>Larry>Stats
    Benternet-->>Client: Tamagotchiland>PetPark?>Larry>Stats>Happiness>100%
    Benternet-->>Client: Tamagotchiland>PetPark?>Larry>Stats>Hunger>100%
    Benternet-->>Client: Tamagotchiland>PetPark?>Larry>Stats>Hygiene>100%

    %% Play
    Client->>Benternet: PUB Tamagotchiland>PetPark!>Larry>Play
    Benternet-->>Client: Tamagotchiland>PetPark?>Larry>Play>Okay I have a number start guessing
    loop Guessing
        Client->>Benternet: PUB Tamagotchiland>PetPark!>Larry>Play>[0-10]
        Benternet-->>Client: Tamagotchiland>PetPark?>Larry>Play>Y/N
    end

    %% Petcare
    Client->>Benternet: PUB Tamagotchiland>PetPark!>Larry>Petcare
    Client->>Benternet: PUB Tamagotchiland>PetPark!>Larry>Petcare>Feeding
    Benternet-->>Client: Tamagotchiland>PetPark?>Larry>Petcare>Thank you for Feeding me!
    Client->>Benternet: PUB Tamagotchiland>PetPark!>Larry>Petcare>Cleaning
    Benternet-->>Client: Tamagotchiland>PetPark?>Larry>Petcare>Thank you for Cleaning me!

    %% Logs
    Client->>Benternet: PUB Tamagotchiland>PetPark!>Larry>Logs
    Benternet-->>Client: [Interaction logs]

    %% Neglect
    alt ❌ Pet neglected too long
        Benternet-->>Client: HEY ATTENTION PLEASE
        Benternet-->>Client: HEY ATTENTION PLEASE
        Benternet-->>Client: HEY ATTENTION PLEASE
        Benternet-->>Client: The pet has died
        Client->>Benternet: PUB Tamagotchiland>PetPark!>Larry
        Benternet-->>Client: The pet has died
    end
```



## Getting Started

To run the project, follow these steps:

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/SamyWarnants/Networkprogramming-Benternetopdracht.git
   ```
2. **Create an account on [Qt group](https://www.qt.io/download-dev) and download the following packages:**
   
   - (For Windows it is setup like this):
     ![Overview](./Resources/qtsetup.png)
3. **Now you can go to the project folder and open the project inside of it.**:
4. **Now you can run the game! Congratulations!! (It can be that the terminal won't let you have input to fix this go to projects=>Run=> and enable run in terminal.**


## People

- **Samy Warnants** - __Student__ - [SamyWarnants](https://github.com/SamyWarnants)

  

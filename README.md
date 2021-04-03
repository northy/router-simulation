<p align="center">
    <img width="400" height="200" src="https://www.google.com.br/url?sa=i&url=https%3A%2F%2Flogo.com%2F&psig=AOvVaw0QnWXG39r3_9-Gr1DOqoOv&ust=1616509723778000&source=images&cd=vfe&ved=0CAIQjRxqFwoTCOjt6JCOxO8CFQAAAAAdAAAAABAD" title="Project Logo"><br />
    <img src="https://img.shields.io/maintenance/yes/2021?style=for-the-badge" title="Project status">
    <img src="https://img.shields.io/badge/platform-linux-lightgray?style=for-the-badge" title="Platform">
    <img src="https://img.shields.io/github/workflow/status/northy/router-simulation/compile?style=for-the-badge" title="Build status">
</p>

# Router Simulation

Router Simulation is a program that simulates routers on a network. The program obtains the configuration information via files. Each router is being able to connect
communicate with other routers (i.e., same program instantiated multiple times)
through UDP sockets.

## Features

* Implemented in C language;
* Developed on the Linux platform;
* Uses UDP sockets only;
* Each NODE is executed as a process (multithreaded);

## Starting

### 1. Dependencies

To execute the project, you will initially need to install the following dependencies:

- [C >=99 compiler](https://gcc.gnu.org/);
- [CMake >=3.0](https://cmake.org/);
- [GNU Make or Ninja](https://www.gnu.org/software/make/);

### 2. Settings

After the installation of the premises, it is necessary to obtain a copy of the project, to do this, clone the project or `fork` it through the button located in the upper right corner and then clone it on your machine.

### 3. Project structure

The project is structured as follows:

| Folder   | Content               |
|----------|-----------------------|
| app/     | Executable files      |
| include/ | Header files          |
| inputs/  | Example input files   |
| src/     | Source routines files |

## Instructions

To clone and compile the repository you need to:

#### GNU Make

```sh
git clone https://github.com/northy/router-simulation.git
cd router-simulation && mkdir build && cd build && cmake ..
make -j
```

#### Ninja

```sh
git clone https://github.com/northy/router-simulation.git
cd router-simulation && mkdir build && cd build && cmake .. -G "Ninja"
ninja
```

### Running the project

Finally, after following the previous steps, to run the project, in the build folder, execute the following command:

```
./Router <id>
```

In the execution folder, there must also contain two configuration files:

#### router.config

In this file, we configure the routers. Each line contains the following:

```
id port ip
```

where:

| Input | Description                  | Restriction                     |
|-------|------------------------------|---------------------------------|
| id    | Router's id                  | id>=1                           |
| port  | UDP socket port to be opened | default port restrictions apply |
| ip    | IPv4 address of the router   | default IPv$ restrictions apply |

#### link.config

In this file, we configure the network bidirectional links. Each line contains the following:

```
id1 id2 cost
```

where:

| Input | Description                 | Restriction          |
|-------|-----------------------------|----------------------|
| id1   | Link's first node           | id1>=1               |
| id2   | Link's second node          | id2>=1, id2!=id1 |
| cost  | Network cost from id1<->id2 | cost>=0              |

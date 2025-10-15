
# 🎬 Movie Logger (GTK + MySQL)

A simple desktop app using **C**, **GTK3**, and **MySQL**.

## Features

* Add movies with Name, Year, Rating
* View saved movie logs
* GUI with basic CSS support

## Compile & Run

```bash
gcc `pkg-config --cflags --libs gtk+-3.0` Movies.c -o movie_app -lmysqlclient
./movie_app
```

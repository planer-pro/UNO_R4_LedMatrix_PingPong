# Пинг-понг на Arduino UNO R4 WiFi

Простая игра "Пинг-понг" для встроенной матрицы платы разработчика Arduino UNO R4 WiFi, реализованная на основе библиотек Adafruit GFX и UNOR4WMatrixGFX.

## Управление
- Ракетка управляется двумя кнопками (влево и вправо) с использованием внутренних подтягивающих резисторов.
- После прохождения всех 10 раундов ты выиграл игру. Если пропустил в одном раунде 10 мячей, то проиграл игру.
и нужно нажать 2 кнопки одновременно для перезапуска игры.
- Вся информация просле окончания раунда или после пропуска мяча отображается на LED матрице бегущей сторокой.

---

# Ping Pong on Arduino UNO R4 WiFi

A simple "Ping Pong" game for the built-in LED matrix of the Arduino UNO R4 WiFi developer board, implemented using the Adafruit GFX and UNOR4WMatrixGFX libraries.

## Controls
- The paddle is controlled by two buttons (left and right) with internal pull-up resistors.
- After completing all 10 rounds, you won the game. If you missed 10 balls in a single round, you lost the game. You need to press 2 buttons simultaneously to restart the game.
- All information (after finishing a round or missing a ball) is displayed as a scrolling text on the LED matrix.
Keyboard consists of 20 buttons (4x4 + 4):
/-----------------------------------\
| an1   | bo2   | cp3   | dq4       |
| er5   | fs6   | gt7   | hu8       |
| iv9   | jw0   | kx({[ | ly)}]     |
| mz,   | Enter | Space | Backspace |
|-----------------------------------|
| Ctrl  | <-    | ->    | Alt       |
\-----------------------------------/

When a multivariational key is pressed (their name starts with lowercase ASCII letter), first of variant is saved and assumed, that pressed first key, but if after 200ms there was a key press to same key, next key is popped (and wrapped to first key if it was last). If there was no key press after 200ms, key is really appendded to buffer. Also this happens, if other key was pressed after first (then first key is pushed to buffer). "Ctrl"/"Alt" keys may be used for faster pressing same key multiple times - press key X, then press "Ctrl" or "Alt", then press key X again.

Variant 2 (3 * 4x4) = 48 keys:
 4x4 button group 1             | 4x4 button group 2            | 4x4 button group 3
/-------------------------------|-------------------------------|-----------------------------------\
| 1 / ( | 2 / ) | 3 / [ | 4 / ] | 5 / { | 6 / } | 7 / ! | 8 / < | 9 / > | 0 / = | Backspace | , / + |
| q / Q | w / W | e / E | r / R | t / T | y / Y | u / U | i / I | o / O | p / P | Space / / | ; / - |
| a / A | s / S | d / D | f / F | g / G | h / H | j / J | k / K | l / L | Enter | ↑         | . / * |
| Ctrl  | Shift | z / Z | x / X | c / C | v / V | b / B | n / N | m / M | ←     | ↓         | →     |
\-------------------------------|-------------------------------|-----------------------------------/

Key "<-" moves cursor to left by 1 symbol, if it on other line, moves it line up and sets cursor to right side. If already in top-level corner, does nothing.

Key "->" moves cursor to right by 1 symbol, if it is right side, moves it line down and sets cursor to left side. If already in bottom-right corner, does nothing.

"Backspace" erases 1 symbol to left from cursor. "Alt" + "Backspace" erases 1 symbol to right from cursor.

If "Enter" is pressed, line is pushed to Belang and if it returns value MULTILINE_CONTINUE, prints "... " prompt for user, to continue line, otherwise line is being executed by Belang.

Commands:
Ctrl + H - Returns cursor to beginning of line
Ctrl + E - Moves cursor to end of line
Ctrl[ + Alt] + Backspace - Erases word
Ctrl + "<-" - Moves cursor one word left
Ctrl + "->" - Moves cursor one word right
Ctrl + L - Prints values of all defined variables
Ctrl + R - Fully resets state (clears screen and current line, undefines all local variables)

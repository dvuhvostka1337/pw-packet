# pw-packet
Perfect World packet sniffer dll

## Build
### Detours
1. Необходима библиотека [ms-detours](https://www.microsoft.com/en-us/download/details.aspx?id=52586)
2. Собирать необходимо под x86
3. В папке ./src открываем x86 Native Tools Command Promt For VS 20..
4. ```nmake all```
5. В VS Studio обозреватель решений жмем свойства ->
   - C/C++ -> Общие (Доп каталоги вкл. файлов) -> путь до папки inlcude
   - Компоновщик -> Общие (доп каталоги библиотек) -> путь до папки lib.X86
   - Компоновщик -> Ввод (доп зависимости) -> путь до файла detours.lib

### Dll
1. Процесс игры x32, поэтому собирать проект необходимо тоже в x32
2. Адреса внутри игры необходимо поменять
```
DWORD address = GameBase + 0x367CE0; // Адрес SendPacket
DWORD BaseAddress = GameBase + 0xA88F70;
```
3. Гайд по [BA](https://progamercity.net/jd/153-poisk-bazovogho-adriesa-smieshchieniia-jade-dynasty.html)
4. Гайд по [SA](https://zhyk.org/forum/showthread.php?t=448589)

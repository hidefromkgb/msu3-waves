msu3-waves
==========

Fourth task for the CG course, GPU-driven water simulation.<br/>
The vast majority of ideas have been borrowed from http://madebyevan.com/<br/>
In fact, this is just a rough port of his «WebGL Water» into C.<br/>
Compatible with Linux, Win32 and WINE.<br/>

(original description in Russian:)

Задание:               4 / OpenGL-симуляция воды

Базовая часть:<br/>
  Наличие пола и стен [+]<br/>
  Объекты под водой   [+]<br/>
  Прозрачность воды   [+]<br/>
  Реалистичный свет   [+]<br/>
  Интерактивность     [+]<br/>
  FPS-независ. обсчёт [+]<br/>

Дополнительная часть:<br/>
  Текстурирование     [+]<br/>
  Отражение           [+]<br/>
  Преломление         [+]<br/>
  Каустика            [+]<br/>
  Управление камерой  [+]<br/>
  Предустановки камер [+]<br/>
  Многопоточность     [+]<br/>

Система:               MinGW / Code::Blocks 10.05<br/>
ОС:                    Windows XP SP2, Windows 7 Home Extended, WINE 1.5.12-1<br/>
Аппаратура:            Core2Duo T5500 1.66 ГГц, 2048 МБ, Radeon X1700 Mobility<br/>

Комментарии:<br/>
Использовались plain C, библиотеки Win32 API (для Windows-версии) и библиотеки
GTK2 и GTKGLext (для Linux-версии). Необходим OpenGL v2.0 или выше.
Вращать камеру можно, зажав левую кнопку мыши, а "бегать" - нажимая W/A/S/D.
Вся геометрия и текстуры генерируются процедурно.

Дополнительный интерактив:<br/>
  - пробел/ПКМ: сбросить на воду каплю<br/>
  - клавиша F1: включить/выключить каркасное отображение<br/>
  - клавиша F2: сменить цветовую палитру<br/>
  - клавиша F3: приостановить / возобновить анимацию<br/>
  - клавиша F4: убрать волнение с поверхности<br/>

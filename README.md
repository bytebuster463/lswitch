# Language switcher

Use any key to switch between input input layouts (informally speaking, 'languages').

The list of switched languages is confined to **only two**
 recent languages even if the system has more than two languages.

For example, your system has the following languages installed:

* English (US)
* Ukrainian
* Chinese (Simplified)
* Thai (Kedmanee)

and the current keyboard layout was *English*,

and the application is called with the following [Parameters](#Parameters):

`lswitch.exe 20 0x409 0xa0`  
(meaning `CapsLock`, `English`, and `LeftShift`).

Then pressing `CapsLock` would cyclically switch between *English* and *Ukrainian*.

Pressing `LeftCtrl + LeftShift` to switch to *Chinese*.
Then pressing `CapsLock` would cyclically switch between *English* and *Chinese*.

Pressing `LeftCtrl + LeftShift` to switch to *Thai*.
Then pressing `CapsLock` would cyclically switch between *English* and *Thai*.


Then pressing `CapsLock` would do:

* if the current language was anything but *English* (e.g. *Thai*),
  `lswitch` would change to *English* and remember *Thai* as 
  last *non-English* layout;
* if the current language was *English* (any layout),
  `lswitch` would change the layout to last remembered
  *non-English* layout (see above).
  * if nothing known about the *non-English* layout
    (e.g. the `lswitch` has just been started),
    the *next available* layout will be chosen;
* if with `LeftShift`, normal `CapsLock` state will change
* 

To chose the *non_English* layout, simply use your Windows standard
hotkey (e.g. `LeftCtrl+LeftShift`). Then 

## Usage

### Synopsis

```
lswitch [keycode [default_layout [guard_keycode]]]
lswitch q
```

### Parameters

Each parameter may take decimal (`42`) or hexadecimal value (`0x123ABCD`).

* `keycode` (optional): `integer`.  
  Which key to use for keyboard switching.  
  Default: `0x5d` context menu key.  
  Another good candidate is a CapsLock key with a keycode of `20`.
* `default_layout`: `ulong`.  
  Default: `0x409` (English)  
  Specifies which language is 'main' for the system
* `guard_keycode`: `ulong`.  
  Default: `(not set)`  
  Which key is to use for 'normal' operation of the `keycode` key.  
  This is useful in combinations like `CapsLock` and `LeftShift`
  so that pressing `CapsLock` would change the layout, but pressing
  `LeftShift + CapsLock` would act as normal `CapsLock` operation.
* `q`: finds the running instance of `lswitch` and lets it close

### Remarks

`lswitch` is intended to have a low memory footprint.
The *Release* version `.exe` is only 5 kb in length.


Forked from [valodzka/lswitch](https://github.com/valodzka/lswitch) .

Adapted [haali.cs.msu.ru/winutils](http://haali.cs.msu.ru/winutils/) .

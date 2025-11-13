# 📈 stocky
[![License: MIT](https://img.shields.io/badge/License-MIT-8affa5.svg)](https://opensource.org/licenses/MIT)

An SDL3 widget that is based on [Theldus/windy](https://github.com/Theldus/windy) that displays live stock information

## Introduction
I've been enjoying Theldus's windy application for a while now for displaying the weather on my desktop.

But i felt that i was missing a equally good looking widget that displays stocks information.

Therefore i made some adjustments and ended up with what i call stocky (super original name lol)!

<p align="center">
<img width="768" height="677" alt="image" src="https://github.com/user-attachments/assets/e1f41b09-7b20-4f10-95e2-8bc9b290e8d2" />
<br>
Stocky examples
</p>
</a>

## How to use
### Quick Explanation:

First of you need a python virtual enviroment for yfinance, easiest way to configure this:
```bash
python -m venv .venv
source .venv/bin/activate
pip install yfinance
```

Change the following line of `stocks_request.py` file with the names of companies seen on Yahoo Finance
```python
# Example
TICKERS = ["MSFT", "NVDA", "AMD", "EA"]
```

and then run:
```bash
(venv)$ ./stocky -t 300 -c "python3 stocks_request.py"
```

### Detailed Explanation:

Unlike most programs/widgets, Stocky doesn't attempt to embed one
or more APIs within itself. Instead, it delegates this responsibility to an
external program/script.

Stocky accomplishes this by invoking an external script provided as a parameter by
the user. This script returns JSON output through stdout, which Stocky reads and
displays on the screen. Essentially, Stocky serves as a GUI for an external
program.

By default, Stocky provides the `stocks_request.py` script, which uses the
[Yahoo Finance's API](https://ranaroussi.github.io/yfinance/) (Handled by yfinance) to fetch weather information.
However, users can supply any script, program, etc., in their preferred language,
as long as it follows the JSON format below:
```json
{
    "longName": "Super duper bear stock",
    "price": 123.45,
    "percentChange": 2.34,
    "provider": "Dummy Data",
    "currency": "USD",
    "sub": [
        {"symbol": "SUB1", "percentChange": -3.2, "price": 140.1, "currency": "SEK"},
        {"symbol": "SUB2", "percentChange": 1.1, "price": 95.2, "currency": "USD"},
        {"symbol": "SUB3", "percentChange": 0.0, "price": 22.2, "currency": "EUR"},
    ],
}

```

### Command-line arguments:

Stocky also supports changing the api update interval (`-t`) and the screen
coordinates (`-x` and `-y`) where it should appear. 
Below are all the available options:
```text
$ ./stocky -h
Usage: ./stocky [options] -c <command-to-run>

Options:
  -t <secs>    Interval (seconds) for stock updates (default: 300)
  -c <command> Command to execute for updates (required)
  -x <pos>     Window X coordinate
  -y <pos>     Window Y coordinate
  -h           Help

Example:
  ./stocky -t 300 -c "python3 stocks_request.py"

Obs: Options -t,-x and -y are not required, -c is required!
```
I don't recommend going under 300 (5 Minutes) due to rate limits, you might get temporarily blocked.

## Building
Stocky requires `SDL3`[^sdl3_note] and `SDL3_ttf` to build, if you don't have
them installed and your distro/OS doesn't have packages for them (highly likely,
as they're still under development) build with CMake:
```bash
$ git clone https://github.com/elch01/stocky.git
$ cd stocky
$ mkdir build
$ cd build
$ cmake ..
$ make -j$(nproc)
```
CMake will check if the libs can be found on the system, and if not, it will
download and build them.

Optionally, if you are running Linux and already have the libraries installed on
your system, just a 'make' is enough:
```bash
$ git clone https://github.com/elch01/stocky.git
$ cd stocky
$ make -j$(nproc)
```

[^sdl3_note]: SDL3 is the first version of SDL to support transparent framebuffer, 
which is why version 3 is required.

## Contributing
Stocky is always open to the community and willing to accept contributions,
whether with issues, documentation, testing, new features, bugfixes, typos, and
etc. Welcome aboard.

## License
Stocky is licensed under the [MIT license](https://opensource.org/license/mit/).

NotoSans font was created by Google and is licensed under the
[OFL license](https://opensource.org/license/ofl-1-1/).

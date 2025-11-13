#!/usr/bin/env python

# MIT License
#
# Copyright (c) 2023 Davidson Francis <davidsondfgl@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import sys
import json
import yfinance as yf

# --- DUMMY DATA FOR TEST MODE ---
DUMMY_OUTPUT = {
    "name": "Super duper bear stock",
    "price": 123.45,
    "percentChange": -2.2,
    "provider": "Dummy Data",
    "currency": "USD",
    "sub": [
        {"symbol": "SUB1", "percentChange": -3.2, "price": 140.1, "currency": "SEK"},
        {"symbol": "SUB2", "percentChange": 1.1, "price": 95.2, "currency": "USD"},
        {"symbol": "SUB3", "percentChange": 0.0, "price": 22.2, "currency": "EUR"},
    ],
}

if "--test" in sys.argv:
    print(json.dumps(DUMMY_OUTPUT, indent=4))
    sys.exit(0)

# --- Edit these for fetching real data, first one is displayed with more detail ---
TICKERS = ["MSFT", "NVDA", "AMD", "EA"]
main_ticker = TICKERS[0]
sub_tickers = TICKERS[1:]

def safe_get(ticker_obj, field, default=None):
    try:
        return ticker_obj.info.get(field, default)
    except Exception:
        return default

def build_output():
    main = yf.Ticker(main_ticker)

    output = {
        "name": (
            safe_get(main, "longName")
            or safe_get(main, "shortName")
            or safe_get(main, "symbol")
            or "N/A"
        ),
        "price": safe_get(main, "regularMarketPrice", 0.0),
        "currency": safe_get(main, "currency", "N/A"),
        "percentChange": safe_get(main, "regularMarketChangePercent", 0.0),
        "provider": "Yahoo Finance",
        "sub": [],
    }

    for ticker in sub_tickers:
        stock = yf.Ticker(ticker)
        output["sub"].append({
            "symbol": ticker,
            "percentChange": safe_get(stock, "regularMarketChangePercent", 0.0),
            "price": safe_get(stock, "regularMarketPrice", 0.0),
            "currency": safe_get(stock, "currency", "N/A"),
        })

    return output

if __name__ == "__main__":
    data = build_output()
    print(json.dumps(data, indent=4))



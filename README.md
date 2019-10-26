# Sunset
[![CodeFactor](https://www.codefactor.io/repository/github/naranbataar/sunset/badge)](https://www.codefactor.io/repository/github/naranbataar/sunset)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/39a80434d93c4c438a929ab070f6db39)](https://www.codacy.com/manual/Naranbataar/Sunset?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Naranbataar/Sunset&amp;utm_campaign=Badge_Grade)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/Naranbataar/Sunset.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/Naranbataar/Sunset/context:cpp)
[![License: Unlicense](https://img.shields.io/badge/license-Unlicense-blue.svg)](http://unlicense.org/)

A simple synthesizer programmable on shell script

## Installation
You need just a C compiler with the default libraries, and make  

## Usage
There are 3 types of inputs  
#### CONFIG (ex: #component key value)  
- Configures component variables    
#### EVENT (@name value)  
The only defailt event is @play, that is handled by generators,   
and accepts seconds or `-` (one frame) as an argument  
#### VALUE  (0.5)  
- Usually PCM audio, numeric input  

Components can be piped together, `sunset` or `sunset COMPONENT` to read more  


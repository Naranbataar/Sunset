# Sunset
A simple synthesizer programmable on shell script

## Installation
You need just a C compiler with the default libraries, and make  

## Usage
There are 3 types of inputs  
CONFIG (ex: #component key value)  
    Configures component variables  
EVENT (@name value)  
    The only defailt event is @play, that is handled by generators,   
    and accepts seconds or `-` (one frame) as an argument  
VALUE  (0.5)  
    Usually PCM audio, numeric input  

Components can be piped together, `sunset` or `sunset COMPONENT` to read more  


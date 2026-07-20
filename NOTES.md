                              """ Shell Written in C """
                              Name For Project: 'OmniShell'

                    --- 12 JULY, 2026. Demetrius Jackson A.K.A OmniKing ---

                                PERSONAL THOUGHTS
  This project will help me strengthen my understanding about how shells are a vital component to an operating system(OS). This is after finishing CS50 and this project shall be the final project I turn in as my final project. First thing is to get a basic understanding of what a shell is.. A shell is a interpreter that allows you to send commands to your computer(OS), it is the translator between the terminal and the operating system itself.

  A shell needs to interpret most common terminal commands and then executes them properly, causing no memory leaks, or race conditions, and more.. I plan by starting with some small simple C code that does some level of functionality to data, like subtracting and adding. I am watching courses on how to start writing your own shell as I go.

                                SHELL IN C
  What is shell written in?
    The most popular shells written like zsh, sh, and most known is bash, are all written in C programming language.
  If shell is written in C, which is a compiled language, then how does compiled C become interpreted shell program?
    Even though shell is interpreted, it is written in C then compiled with knowing how to parse text into operating system actions.

                                WHAT SHELL CREATES
  Shells create PID's or PPID's (Process Identifier) which is the process that holds the execution of a program in memory. They stay running until the PID is killed. Shell is already in use the second your computer boot on, storing all background process with an ID so then the operating system can keep track and it allows users to interact with the specific PID or PPID. A Process is basically a program in execution.


                            --- 13 JULY, 2026. OmniKing ---

                                PERSONAL THOUGHTS
  After having ventured into what it means to build a shell written in C is to start by looking at those that have already done it. I learn better seeing the code visually while someone who understands the subject is explaining as he builds it live. So far I learned about new functions and terminology that will help me better read documentation and know how to quickly implement the guidance provided by documentation.

  Today I tackle some code that allows me to see how shells commands are done, by creating PID's then learning how to understand the status codes that certain functions provide. Building header files to learn how to break down my code into cleaner broken up files. I will finish a starting version of the shell today but only after I get some practice in.

                                WHAT SHELL FUNCTIONS ARE USED
  What does the function 'fork' provide and why is it important to shell?
      Fork is a function that creates a Child Process. A child process is a new process that duplicates the parent process.
  What purpose does shell serve?
      Shell serves the purpose for users to interact with their operating system using either built-in functions, or by parsing the command into a tree of nodes that get executed as child processes.

                                LEARNING FROM DONE PROJECTS
  Taking the time to watch tutorials and lectures on what a shell is on a fundamental level. Has granted me more confidence to tackle today's challenge, which is to create a blueprint for the functionalities my shell will incorporate so then I can start to build a tree in mind of what the process should look like as I build.


                            --- 15 JULY, 2026. OmniKing ---

                                PERSONAL THOUGHTS
  After even more research I found my confidence getting stronger on knowing that I can build a shell following just documentation tutorials. Currently the source of material that I chose is, 'Tutorial - Write a Shell in C' by Stephen Brennan. Easy to read and follow, I found learning more about the foundations of building a shell is what is needed. Not some guide that skips a lot of explaining why certain functions or files are being organized in a specific way. Now I found what is helping me sharpen my questions so then solutions can start brewing in my mind. The fun lies in the learning process, not the performance of what you learned.

                            WHY USE A MANUAL INSTEAD OF FINISHED PRODUCTS
  How will I start my OmniShell?
    Starting my scripts with the blueprints and basic files to kick off writing code in C so picturing the finished product as I go starts to mold.
    So far 3 scripts are made, one being the main shell script, then the other two being header files. I can understand all the code I wrote and why it is written a certain way. The goal is to also add comments everywhere so then the future me can understand why I wrote code a specific way in the past.

  Why was I ignoring the use of AI even when this final project encourages to do so?
    It's not that AI is not useful, especially when you know the code yourself, but it is hard for me to leave some of the research to AI. I am pretty picky about how information is presented to me. Since taking this online course, at first it felt easy than it got challenging. Sitting there struggling for 5 hours on a function for something as trivial as comparing a single character to other character through arrays of text. But finally achieving that goal, noticing my mind slowly grasp the entirety of my code so than I can finally debug it only using the knowledge taught in the course.
    I told myself since that enjoyment of overcoming a learning curve felt powerful and slightly addicting to me LOL. Now I want to keep throwing those hurdles at myself since the impact it grants you afterwards is way more valuable than AI giving me an answer. But I do plan on using AI for this project, in order to teach myself how I can get AI to work for how I like to learn and program.
    Using Chatgpt has been my source, mainly for research but struggling to get the information needed to understand, not just an answer to my problem. Teaching gpt how I learn and to not take it away has been working, especially after spending the $20 for the pro version eventhough I am far from exceeding my token limit, it's for the fact I like to send screenshot instead of text, those images per context window are limited, so this Pro allows "unlimited" files I can send.


                            --- 18 JULY, 2026. OmniKing ---

                                PERSONAL THOUGHTS
  Having taken a couple days to look over the article by Stephen Brennan. I can confidently say that I understand what the base foundations of what a shell is, and how it is supposed to operate in normal daily use. Learning a lot from the fundamentals of a shell has given me the idea that I believe solves a problem that I deal with and other developers might as well.
 Looking back from when I started learning how to program for the first time, which was taking this course. When programming different languages, you have to remember all these sequence of commands just to launch/run a program. Noticing the name 'omni', I thought, why not just let the shell determine what missing requirements are missing from trying to run/launch a program like C, Python, C++, HTML files, and more!

  So that is the goal for this project that I believe solves a problem that would inspire me to turn this project into a real project, this is the time I am really taking to learn how to read documentation and write the code myself based on what I read and research, still learning how to program mentally. Using AI here and there to just clean up some errors I miss and also asking if I have used a function improperly. That to me is enough use of AI while I am still a student to this new field I am entering.

  Currently using chatgpt to talk about the architecture and how to implement my vision into my shell, today I have finished reading the article researching in-between to fully grasp the teachings the article offered. Now have built a shell of my own using the skeleton code the article provided.

                                WHAT IS A SHELL
  What is a shell?
      A shell is a program that allows the user to command the operating system. By using a child process, you can run another program inside a program like a shell which is the parent process. The child would run the command that the operating system provided. You can also write builtin commands that allow the user to not rely on the operating system to have said program.


                            --- 19 JULY, 2026. OmniKing ---

1. Write a short essay (half a page or so) on software development in c++, including what you think are important good practices and why, some practices that you avoid and why, and how you like to work and fit in with a team. (And you may like to start the next item first, and write the essay once you have started it building). (Answer in Spanish if you prefer)

--------

Overall you can use Alan Kay as an inspiration. Basically any software development can be done as follows:
1. Determine inputs/outputs(everything, even video games are functions that transform data).
2. Write the general data transformation function required for your program. (e.g. player input -> rendered frame)
3. Check whether your function can be implemented in a programming language already
4. If not subdivide the function into subprograms. (e.g. monster health system, player movement system, etc) Subprograms will communicate via messages to facilitate the general data transformation performed by your function.
5. Recursively do 1 through 4 for each subprogram until you only have functions.
6. Write dummy functions and tests for your program/subprograms; delegate parts of this work to other team members.
7. Everyone in your team implements functions until you have fully functioning software.

Now, facilitating teamwork, writing initial specification for your software domain and how it actually transforms data, and what data, is an art and a science. But in general, this is how you develop software in a controlled fashion. You don't search for a cool design paradigm to implement, you don't try to eyeball it, you define the data that you'll work with(aka business requirements), you design the execution flow, you write down api(ideally as tests), and then you implement functionality. This will save you tremendous amounts of time and reworks, and will enable every one of your programmers to work meaningfully. Even should you require a rework due to change in requirements, you will only have to rewrite some subset of your functions/subprograms, or, if you need to completely change how program works, you will be able to reuse your already implemented functions and subprograms(or you can call them modules). With a monolithic design that was conforming to, say, MVC, you'll have a dead codebase and all your good programmers will resign, because no one will one to rewrite everything. Then either someone WILL rewrite everything, or your company goes bankrupt.

As for c++ specifically, just use Rust or copy Rust. Rust strongly penalizes premature optimisation and low-level primitive abuse, which are big parts of c++ culture. So if you want to implement something, but you can't use Rust, just see how it's implemented in Rust. Also, it's up to management and teamleads to not force programmers to constantly prove themselves via overuse of low level crap that no one will be able to debug later. Remember that code is written once, then read a 1000 times. Also it pays to remember that we, programmers, are not selling a solution. Company as a wholes sells solution. We sell codebase.

Don't be afraid to implement commonly used functionality as a subprogram/function/primitive. Something like channels for example should be in the codebase, if you have condvars in your general code you're either designing the most performant software in your domain, or you're doing it wrong.

To sum up, good practices are top-down development, modular subprograms, immutable code. Bad practises are premature optimisations and use of low level primitives before profiling has been done.

I like to work in a team where everyone knows what they're doing, what the company as a whole is doing, and where you have clear requirements and communications from management to engineering. No one should be working on features beyond MVP granularity, if these features may be abandoned. It's up to senior management and teamleads to make everyone aware of what features may be abandoned and which long term decisions have been made.

--------

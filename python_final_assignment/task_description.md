Using the data available on the website [https://api.um.warszawa.pl/#](https://api.um.warszawa.pl/#), collect information about the positions of buses within a specified time range. It is suggested to choose two such intervals of at least one hour each (an interesting comparison might be one of the peak traffic hours compared to early morning or late evening).

Then, conduct an analysis of the collected data. You are encouraged to use your creativity, formulate your own research hypotheses, and verify them using the collected data.

The questions that need to be answered, regardless of your own analyses, are:

**Speeding**
1.1) How many buses exceeded the speed of 50 km/h?
   - Note: The bus position is updated every minute. Assuming the bus moves in a straight line during that minute, we can approximate the actual speed.
1.2) Identify if there were places where a significant percentage of buses exceeded this speed limit.
   - Note: In the solution, you should define the concept of a location, e.g., it could be a specific place in the city such as a street or bridge, or a radius around a given geographical point.

**Punctuality**
   - Analyze the punctuality of buses during the observed period. Compare the actual arrival times at stops with the scheduled timetable.

**Technical requirements, grading guidelines, and helpful notes:**

**A)** Implement the solution in two parts. The first for collecting data and saving it to a file. The second for analyzing the collected data. This allows for the flexibility of exchanging one part of the solution for another as data collection is time-consuming.

**B)** Grading will consider:
   - Explanation of design decisions during the exam.
   - Proper division of code into packages and modules.
   - Code quality with adherence to PEP 8 standards and use of tools like pylint or flake for code checking.
   - Attention to the proper naming of functions and variables with an emphasis on short functions without repetitive code fragments.
   - The ability to install the code as a package using pip install.
   - Code coverage with tests.
   - Analysis of the program with a profiler to discuss bottlenecks, with a focus on analysis and discussion over code iteration.
   - Presentation method (if a script, does it use argparse; if a Jupyter notebook, how are the data visualized). A clear project description.
   - Is there any original analysis in addition to the mandatory ones.
   - Data visualization and optional mapping of results in Warsaw.

**C)** Helpful notes include the recommendation of numpy and pandas libraries for data analysis, which will be covered in the first and second lessons in 2024. The third lesson in January will focus on visualization.

**D)** Deadline for submission:
   - To be determined with the lab instructor.
   - The first step is to send access to the repository and information about the last commit to the instructor.
   - Then arrange a specific date to present the program and analysis results to the instructor.
   - If graded before the end of the session, the grade will be recorded at the first opportunity.
   - Those who receive a grade after the end of the session but before the end of the makeup session will receive a grade at the second opportunity.
   - Note, the last day the instructor may have booked slots. Please also consider that there is a need for time to familiarize with your project before presentation.

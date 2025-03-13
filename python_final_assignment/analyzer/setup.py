from setuptools import setup, find_packages


# Function to read the list of dependencies from requirements.txt
def parse_requirements(filename):
    """ Load requirements from a pip requirements file """
    with open(filename, 'r') as file:
        lines = (line.strip() for line in file)
        return [line for line in lines if line and not line.startswith("#")]


# Call the function and assign the result to a variable
install_requires = parse_requirements('requirements.txt')

setup(
    name='data_collector',
    version='0.0.1',
    packages=find_packages(),
    install_requires=install_requires,
    # other arguments omitted
)

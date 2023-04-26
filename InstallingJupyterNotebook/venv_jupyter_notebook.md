# Jupyter Notebooks and Virtual Environments
I recommend learning to analyze your data using Jupyter Notebooks, at least for prototyping. Furthermore, I recommend you do all your analysis for each project in its own “virtual environment”, which makes it easier to keep track of the versions of packages you are using. This can be easily accomplished through tools like Anaconda, however, Anaconda and ROS do not get along. So, we will take a few extra steps and do it from scratch.

There are many instructions out there.. but also many pitfalls. These instructions, will _hopefully_ work.

ROS runs on python 2.7 (we have not yet migrated to ROS2). But I recommend your analysis be done in python 3. I see no reason to not use python 3.8.

Assumptions:
* Ubuntu 18+
* Python 3.8
* You have some basic command of ubuntu and the terminal

# Install python and create virtual environment

1. Check your version of python, if you don’t have 3.8, install it: [https://docs.python-guide.org/starting/install3/linux/](%22)

 
2. Create a virtual environment

   <code>python3.8 -m venv /path/to/new/virtual/environment</code>
  for example:
  <code>python3.8 -m venv ~/ANALYSIS_38_TEST</code>
  
    You may encounter the following error:
   <code>Error: Command '['/home/caveman/ANALYSIS_38_TEST/bin/python3.8', '-Im', 'ensurepip', '--upgrade', '--default-pip']' returned non-zero exit status 1.</code>

    If you do, try this:
    <code>python3.8 -m venv ~/ANALYSIS_38_TEST --without-pip</code>

3. Activate your virtual environment:
	<code>source ~/ANALYSIS_38_TEST/bin/activate</code>
	You should now see <code>(ANALYSIS_38_TEST)</code> at the beginning of your command prompt, which means you are in the virtual environment. 

4. Make sure you have the right python version, run python:
	<code>python</code>
	You should see _python 3.8.X_, e.g. _3.8.3_. 
	
5. Exit python:
	<code>ctrl-d</code>

6. Get pip working in your virtual environment. The problem with adding "--without-pip" is that pip is very convenient for installing python packages. So, we will have to install it into the virtual environment manually. All the following should be done with your virtual environment active.
	* You might need to run this: 
	<code>sudo apt-get install python3.8-distutils</code>
	* Then download a python script with the following command:
	<code>curl https://bootstrap.pypa.io/get-pip.py | python</code>
	* And run it:
	<code>python ~/get-pip.py</code>
	* Make sure you're up to date:
	<code>python3.8 -m pip install --upgrade pip</code>

7. Test out pip, and install numpy.
	* Check to see if numpy is installed in your virtual environment, it should *not* be. If it is, you might not have activated your virtual environment?
	<code>python</code> 
	<code>>>> import numpy</code> should give an error (the >>> indicates this is in the python interpreter, you should not type this)
	<code>ctrl-d</code> exit python interpreter
	* Install numpy. There are many different syntaxes for using pip to install packages. This is the most verbose, but most explicit. I recommend it.
	<code>python3.8 -m pip install numpy</code>
	* Repeat step a, you should have numpy installed now. 

8. Optional: Installing a specific version of a package. Perhaps you need to run code that uses an old version of numpy. This is the power of virtual environments, you can have multiple versions of the same package installed in different virtual environments. 
   * What version is installed?
   <code>python</code>
   <code>>>> import numpy</code>
   <code>>>> numpy.&#95;&#95;version__</code> mine shows 1.19
   <code>ctrl-d</code> exit python interpreter
   * Uninstall current version
	<code>python3.8 -m pip uninstall numpy</code>
   * Install a specific version:
   <code>python3.8 -m pip install numpy==1.18</code>
   
# Installing a bunch of packages from a list of requirements
If you have a list of requirements, like requirements.txt (for example: https://github.com/florisvb/PyNumDiff/blob/master/requirements.txt), you can install all of them at once using the command:
<code>python3.8 -m pip install -r REQUIREMENTS.TXT</code>

# Install Jupyter Notebook into your virtual environment
1. Install jupyter notebook (with your virtual environment active):
<code>python3.8 -m pip install jupyter notebook</code>

2. Add your virtual environment to jupyter notebook (with your virtual environment active):
<code>python3.8 -m ipykernel install --user --name=ANALYSIS_38_TEST</code>

3. Run jupyter notebook (with your virtual environment active):
<code>jupyter notebook</code>

4. A browser window should open with a file browser like view. If it doesn't, you can access it manually.
  * Take note of this line:
<code>https://[all ip addresses on your system]:10003/</code> The last number is your notebook's port. But you will need to know your IP address. 
  * Finding your IP. Open a terminal window, type:
<code>ifconfig</code>
Look for something like this: <code>inet addr:134.197.27.23</code>
  * Open firefox (it is the most robust).
  * Go to:
<code>https://your_ip_address:your_port</code>
for example:
<code>https://134.197.27.23:10003</code>
	
5. Open a notebook. Click <code>File>New Notebook..."</code>. Choose the one that matches your virtual environment name.

6. Make sure you're running the right version of python:
        * <code>import sys</code>
        * <code>print(sys.version)</code>

7. Make sure that things are working properly. 
	* <code>import numpy</code> should work
	* <code>import matplotlib</code> should not work - we'll fix that in the next section
	
# Install some packages that we will commonly use
1. With the virtual environment active, in the terminal, run:
<code>python3.8 -m pip install scipy pandas tables matplotlib h5py</code>
2. Go back to your jupyter notebook. Go to <code>Kernel<Restart</code>. 
3. <code>import matplotlib</code> should work now

# Learn some notebook basics.
For example: https://realpython.com/jupyter-notebook-introduction/

# Managing your notebooks
1. When you're done with a jupyter notebook session, <code>ctrl-c</code> in the terminal where that notebook is active to kill it.
2. Can't remember what's running?
   <code>jupyter notebook list</code>
3. Can't find the terminal responsible for a notebook session, and you want to kill it? 
    * Take note of the address:port from the list. e.g. <code>https://localhost:10002/</code>
    * Find the responsible process
    <code>ps aux | grep https://localhost:10002/</code>
    That should show something like:
      * <code>username 5181  0.0  0.0  14224  1012 pts/32 S+ 14:34 0:00 grep --color=auto **https://localhost:9999/**</code>
    * Take note of the process number, after the username, e.g. <code>5181</code>   
    * Kill it.
      * <code>sudo kill 5181</code>
# Optional: running a remote jupyter notebook server

If you wish to run your notebook on a remote computer, you can set up a password etc. Most of the lab desktops already have this set up, but for reference: [https://jupyter-notebook.readthedocs.io/en/stable/public_server.html](https://jupyter-notebook.readthedocs.io/en/stable/public_server.html)

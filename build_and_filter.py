import subprocess
import re

def run_build():
    result = subprocess.run(['idf.py', 'build'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    process_output(result.stdout)
    process_output(result.stderr)

def process_output(output):
    lines = output.split('\n')
    for line in lines:
        if 'undefined reference to' in line:
            match = re.search(r'undefined reference to `(.*)`', line)
            if match:
                print(f"ld.exe: {match.group(0)}")
        else:
            print(line)

if __name__ == "__main__":
    run_build()

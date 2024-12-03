# Lambda Mail

## Setup

1. Install Conda if you haven't already:
   - Download from [https://docs.conda.io/en/latest/miniconda.html](https://docs.conda.io/en/latest/miniconda.html)
   - Or install with: `brew install --cask miniconda` (on macOS)

2. Activate the environment:
```bash
conda activate lambdamail
```

3. Run the setup script:
```bash
./setup.sh
```

4. Build the project:
```bash
mkdir build
cd build
cmake ..
make
```

The executables will be in the `build/bin` directory.
- **./get_domain**: 
      - shows the functionality of fetching a domain from the API 
- **./register_email**: 
      - allows you to create a temporary email and password 
- **./check_inbow**:
      -  allows you to log into your email using the credentials created in the register_email and view the emails you recieve in your inbox and delete the account when needed
- **./mail_client**:
      -  allows you to test out all these features with an interactive GUI



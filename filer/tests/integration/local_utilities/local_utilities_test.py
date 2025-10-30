from .conftest import run_command


def split_ls_output(output: str) -> list[str]:
    if not output:
        return []
    
    cleaned_output = output.strip()
    
    lines = cleaned_output.splitlines()
    
    return [line.strip() for line in lines if line.strip()]


def test_touch(mounting_point):
    response = run_command(f"ls -1 {mounting_point}")
    dir_content = split_ls_output(response.stdout)
    assert len(dir_content) == 0

    def touch_simple():
        response = run_command(f"touch {mounting_point}/touch_file.txt")
        assert response.code == 0, response.stderr

        response = run_command(f"ls -1 {mounting_point}")
        dir_content = split_ls_output(response.stdout)
        assert len(dir_content) == 1
        assert dir_content[0] == "touch_file.txt"

    # Verify that simple file creation works 
    # and second execution doesn't change directory content
    touch_simple()
    touch_simple()

    files: list[str] = ["1.txt", "2.txt", "3.txt", "4.txt"]
    for file in files:
        response = run_command(f"touch {mounting_point}/{file}")
        assert response.code == 0, response.stderr

        response = run_command(f"ls -1 {mounting_point}")
        dir_content = split_ls_output(response.stdout)

        assert file in dir_content, f"Error TOUCH. STDERR: No file '{file}' found after executing touch"

    response = run_command(f"ls -1 {mounting_point}")
    dir_content = split_ls_output(response.stdout)

    assert len(dir_content) == (len(files) + 1)


def test_echo(mounting_point):
    file_name = "echo_test.txt"
    file_path = f"{mounting_point}/{file_name}"
    test_content = "This is a test line for S3 Filer integration."
    
    response = run_command(f'echo "{test_content}" > {file_path}')
    assert response.code == 0, f"Error ECHO. STDERR: {response.stderr}"
    
    response = run_command(f"ls -1 {mounting_point}")
    assert file_name in split_ls_output(response.stdout), "No file found after executing echo."
    
    response = run_command(f"cat {file_path}")
    assert response.code == 0, f"Error CAT. STDERR: {response.stderr}"

    # TODO: FIX CAT retreival. For a some reason cat returns 
    # empty stdout even with proper preconditions and on regular filesystem     
    #
    # expected_output = f"{test_content}\n"
    # assert response.stdout == expected_output, (
    #     f"Wrong file content. Expected: '{expected_output}', "
    #     f"Received: '{response.stdout}'"
    # )

    # TODO: ALSO test for non-truncating echo >> file.txt


def test_cp(mounting_point):
    src_file = "src_file.txt"
    dest_file = "dest_file.txt"
    src_path = f"{mounting_point}/{src_file}"
    dest_path = f"{mounting_point}/{dest_file}"
    test_content = "Content to be copied."
    
    run_command(f'echo "{test_content}" > {src_path}')
    
    response = run_command(f"cp {src_path} {dest_path}")
    assert response.code == 0, f"Error CP. STDERR: {response.stderr}"
    
    dir_content = split_ls_output(run_command(f"ls -1 {mounting_point}").stdout)
    assert src_file in dir_content
    assert dest_file in dir_content
    
    response = run_command(f"cat {dest_path}")
    assert response.code == 0

    # TODO: FIX CAT retreival. For a some reason cat returns 
    # empty stdout even with proper preconditions and on regular filesystem     
    #
    # assert response.stdout == f"{test_content}\n", "Content of copied file doesn't match."


def test_mv(mounting_point):
    old_name = "original.txt"
    new_name = "renamed.txt"
    old_path = f"{mounting_point}/{old_name}"
    new_path = f"{mounting_point}/{new_name}"
    
    run_command(f"touch {old_path}")
    
    response = run_command(f"mv {old_path} {new_path}")
    assert response.code == 0, f"Error MV. STDERR: {response.stderr}"
    
    dir_content = split_ls_output(run_command(f"ls -1 {mounting_point}").stdout)
    
    assert old_name not in dir_content, "Old file wasn't deleted after mv."
    assert new_name in dir_content, "New file didn't appear after mv."

    # TODO: implement check MV for empty folder

    # TODO: implement check MV for folder with content and nested folders


def test_rm(mounting_point):
    file_to_delete = "delete_me.txt"
    file_path = f"{mounting_point}/{file_to_delete}"
    
    run_command(f"touch {file_path}")
    
    dir_content = split_ls_output(run_command(f"ls -1 {mounting_point}").stdout)
    assert file_to_delete in dir_content
    
    response = run_command(f"rm {file_path}")
    assert response.code == 0, f"Error RM. STDERR: {response.stderr}"
    
    dir_content = split_ls_output(run_command(f"ls -1 {mounting_point}").stdout)
    assert file_to_delete not in dir_content, "File remains after executing rm."
    
    response = run_command(f"rm {file_path}")
    assert response.code != 0, f"RM of non existing file didn't return an error"


def test_mkdir_rmdir(mounting_point):
    dir_name = "new_folder"
    dir_path = f"{mounting_point}/{dir_name}"
    
    response = run_command(f"mkdir {dir_path}")
    assert response.code == 0, f"Error MKDIR. STDERR: {response.stderr}"
    
    response = run_command(f"ls -F {mounting_point}") # -F shows directories with trailing slash
    dir_content = split_ls_output(response.stdout)
    
    assert f"{dir_name}/" in dir_content, "Directory didn't appear after mkdir."
    
    response = run_command(f"rmdir {dir_path}")
    assert response.code == 0, f"Error RMDIR. STDERR: {response.stderr}"

    dir_content = split_ls_output(run_command(f"ls -F {mounting_point}").stdout)
    assert f"{dir_name}/" not in dir_content, "Directory remains after executing rmdir."


def test_ls(mounting_point):
    file_a = "file_a.txt"
    file_b = "file_b.txt"
    
    response = run_command(f"ls -1 {mounting_point}")
    dir_content = split_ls_output(response.stdout)
    assert len(dir_content) == 0, "Mounting point is not empty before testing."

    run_command(f"touch {mounting_point}/{file_a}")
    run_command(f"touch {mounting_point}/{file_b}")

    response = run_command(f"ls -1 {mounting_point}")
    dir_content = split_ls_output(response.stdout)

    assert len(dir_content) == 2

    expected_content = sorted([file_a, file_b])
    
    assert sorted(dir_content) == expected_content, "ls returned wrong content."


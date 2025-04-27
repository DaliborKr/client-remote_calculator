import filecmp
import os


number_of_tcp_tests = 3
number_of_udp_tests = 1
host_ip = "0.0.0.0"
port = "2023"

i = 1
while i <= number_of_tcp_tests:
    in_file = "tests/tcp/test_tcp_input" + str(i) + ".in"
    out = "tests/tcp/test_tcp_output" + str(i) + ".out"
    expected = "tests/tcp/test_tcp_expected_output" + str(i) + ".out"

    command = "./ipkcpc -h " + host_ip + " -p " + port + " -m tcp < " + in_file + " > " + out
    os.system(command)

    result = filecmp.cmp(out, expected, shallow=False)
    if (result):
        print("test_tcp" + str(i) + " : OK")
    else:
        print("test_tcp" + str(i) + " : ERR\nActual program output: " + out + "\nExpected program output: " + expected + "\n\nActual program output and expected program output does not match")
    
    
    i += 1

i = 1
while i <= number_of_udp_tests:
    in_file = "tests/udp/test_udp_input" + str(i) + ".in"
    out = "tests/udp/test_udp_output" + str(i) + ".out"
    expected = "tests/udp/test_udp_expected_output" + str(i) + ".out"

    command = "./ipkcpc -h " + host_ip + " -p " + port + " -m udp < " + in_file + " > " + out
    os.system(command)

    result = filecmp.cmp(out, expected, shallow=False)
    if (result):
        print("test_udp" + str(i) + " : OK")
    else:
        print("test_udp" + str(i) + " : ERR\n\tActual program output: " + out + "\n\tExpected program output: " + expected + "\n\n\tActual program output and expected program output does not match!")
    
    i += 1
 
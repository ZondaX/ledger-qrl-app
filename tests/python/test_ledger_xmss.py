from __future__ import print_function

import binascii
import sys
import time

import pytest

from tests.python.known_values import expected_leafs_zeroseed
from tests.python.pyledgerqrl import ledgerqrl
from tests.python.pyledgerqrl.ledgerqrl import INS_VERSION, INS_TEST_PK_GEN_1, INS_TEST_PK_GEN_2, INS_TEST_READ_LEAF, \
    INS_TEST_PK, INS_TEST_SIGN, INS_TEST_DIGEST


def test_version():
    """
    Verify the tests are running in the correct version number
    """
    answer = ledgerqrl.send(INS_VERSION)
    assert len(answer) == 4
    assert answer[0] == 0xFF
    assert answer[1] == 0
    assert answer[2] == 0
    assert answer[3] == 2


def test_pk_gen_1():
    """
    Test key generation phase 1
    """
    answer = ledgerqrl.send(INS_TEST_PK_GEN_1)
    assert len(answer) == 132

    idx = binascii.hexlify(answer[0:4]).upper()
    seed = binascii.hexlify(answer[4:36]).upper()
    prf_seed = binascii.hexlify(answer[36:68]).upper()
    pub_seed = binascii.hexlify(answer[68:100]).upper()
    root = binascii.hexlify(answer[100:]).upper()

    print(len(answer))
    print(seed)
    print(prf_seed)
    print(pub_seed)

    assert seed == "EDA313C95591A023A5B37F361C07A5753A92D3D0427459F34C7895D727D62816"
    assert prf_seed == "B3AA2224EB9D823127D4F9F8A30FD7A1A02C6483D9C0F1FD41957B9AE4DFC63A"
    assert pub_seed == "3191DA3442686282B3D5160F25CF162A517FD2131F83FBF2698A58F9C46AFC5D"


def test_pk_gen_2():
    """
    Test key generation phase 2
    """
    answer = ledgerqrl.send(INS_TEST_PK_GEN_2, [0, 0])
    assert answer is not None
    assert len(answer) == 32
    leaf = binascii.hexlify(answer).upper()
    print(leaf)
    assert leaf == "98E68D7AB40D358B5B0F4DF4C86AAE78B444BD50248C02773CF1965FAEA092AE"
    sys.stdout.flush()

    answer = ledgerqrl.send(INS_TEST_PK_GEN_2, [0, 20])
    assert len(answer) == 32
    leaf = binascii.hexlify(answer).upper()
    print(leaf)
    assert leaf == "8E66C0B26238BC9E12804A83AEF0429E9A666266001A826B5025889B45AE86A3"
    sys.stdout.flush()

    answer = ledgerqrl.send(INS_TEST_PK_GEN_2, [0, 40])
    assert len(answer) == 32
    leaf = binascii.hexlify(answer).upper()
    print(leaf)
    assert leaf == "D2BAD383B25900503A34FA126ABB19D3AAC6FC110F431929C7EB18E613E101F8"
    sys.stdout.flush()


@pytest.mark.skip(reason="This test takes around 42 mins and generates all the keys")
def test_pk_keys():
    """
    Generate all leaves (256)
    """
    assert len(expected_leafs_zeroseed) == 256
    start = time.time()
    for i in range(256):
        answer = ledgerqrl.send(INS_TEST_PK_GEN_2, [0, i])
        assert len(answer) == 32
        leaf = binascii.hexlify(answer).upper()
        assert leaf == expected_leafs_zeroseed[i]
        end = time.time()
        print(end - start, leaf)
        sys.stdout.flush()


@pytest.mark.skip(reason="This test is only useful when leaves are all zeros")
def test_read_leaf_all_zeros():
    """
    Read all leaves and checks that they are zero
    """
    start = time.time()
    for i in range(1, 256):
        answer = ledgerqrl.send(INS_TEST_READ_LEAF, [i])
        assert len(answer) == 32
        leaf = binascii.hexlify(answer).upper()
        assert leaf == "0000000000000000000000000000000000000000000000000000000000000000"


@pytest.mark.skip(reason="This test is only useful when leaves are all zeros")
def test_pk_when_all_leaves_are_zero():
    """
    Get public key when all leaves are zero
    """
    answer = ledgerqrl.send(INS_TEST_PK, [])
    assert len(answer) == 64
    leaf = binascii.hexlify(answer).upper()
    print(leaf)
    assert leaf == "EB2920D87FB8D5C4C32A9F7D0F33A3AA4C7D044B77EE6260683ABFA2111E5A18" \
                   "0000000000000000000000000000000000000000000000000000000000000000"


def test_read_leaf():
    """
    Expects all leaves to have been generated or uploaded. It compares with known leaves for the test seed
    """
    assert len(expected_leafs_zeroseed) == 256
    start = time.time()
    for i in range(256):
        answer = ledgerqrl.send(INS_TEST_READ_LEAF, [i])
        assert len(answer) == 32
        leaf = binascii.hexlify(answer).upper()
        assert leaf == expected_leafs_zeroseed[i]


def test_pk_when_leaves_exist():
    """
    Expects all leaves to have been generated or uploaded. It checks with a known public key for the test seed
    """
    answer = ledgerqrl.send(INS_TEST_PK, [])
    assert len(answer) == 64
    leaf = binascii.hexlify(answer).upper()
    print(leaf)
    assert leaf == "106D0856A5198967360B6BDFCA4976A433FA48DEA2A726FDAF30EA8CD3FAD211" \
                   "3191DA3442686282B3D5160F25CF162A517FD2131F83FBF2698A58F9C46AFC5D"


def test_digest_idx_5():
    """
    Checks the message digest for an all zeros message
    """
    msg = bytearray([0] * 32)
    assert len(msg) == 32
    index = 5
    params = bytearray([index]) + msg
    assert len(params) == 33

    answer = ledgerqrl.send(INS_TEST_DIGEST, params)
    answer = binascii.hexlify(answer).upper()
    print(answer)

    assert answer == "CEAB1D37701A1FCB8BD5F78F23396104BF05BF07761E4E1C9EA2E10A158A54FD" \
                     "743EF66B8257AF7BCCF1197C4B93CDCFC6EC805A408841735F80150885A2D60D"

def test_digest_idx_25():
    """
    Checks the message digest for an all zeros message
    """
    msg = bytearray([0] * 32)
    assert len(msg) == 32
    index = 25
    params = bytearray([index]) + msg
    assert len(params) == 33

    answer = ledgerqrl.send(INS_TEST_DIGEST, params)
    answer = binascii.hexlify(answer).upper()
    print(answer)

    assert answer == "584C7CDDE280E4112F040D323DF445A3A4C77F66CA359EC59275A42B2AD8774D" \
                     "850B8D1BDB605346FACBA48A17D37F4484A6C046B54E6EAA83C37850DEC59001"

def test_sign():
    """
    Sign an empty message
    """
    msg = bytearray([0] * 32)
    assert len(msg) == 32
    index = 0
    params = bytearray([index]) + msg
    assert len(params) == 33

    answer = ledgerqrl.send(INS_TEST_SIGN, params)
    print(answer)

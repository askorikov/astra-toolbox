import pytest


def pytest_addoption(parser):
    parser.addoption(
        '--skip-backends', default=[], action='extend', nargs='*',
        help="Skip tests for specific backends"
    )


def pytest_collection_modifyitems(config, items):
    backends_to_skip = config.getoption('--skip-backends')
    for item in items:
        if item.callspec.params.get('backend') in backends_to_skip:
            item.add_marker(pytest.mark.skip('Backend skipped according to command line args'))

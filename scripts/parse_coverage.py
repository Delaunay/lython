
import xml.etree.ElementTree as ET
import argparse
import csv

parser = argparse.ArgumentParser()
parser.add_argument('--report')
parser.add_argument('--csv')
parser.add_argument('--template')
parser.add_argument('--target')
args = parser.parse_args()

# Append coverage to the database
report = ET.parse(args.report)
root = report.getroot()
newrow = root.attrib

cols = []
rows = []

def append_unique(array, elts):
    for elt in elts:
        if elt not in array:
            array.append(elt)


with open(args.csv, encoding='utf-8', newline='') as csvfile:
    reader = csv.DictReader(csvfile)

    for row in reader:
        append_unique(cols, row.keys())
        rows.append(row)


append_unique(cols, newrow.keys())

with open(args.csv, 'w', newline='') as csvfile:
    writer = csv.DictWriter(csvfile, fieldnames=cols)
    writer.writeheader()

    for row in rows:
        writer.writerow(row)

    writer.writerow(newrow)

# Generate the badge coverage
coverage = float(newrow['line-rate'])

with open(args.template, encoding='utf-8', newline='') as file:
    template = file.read()

template = template.replace('COVERAGE_VALUE', f'{coverage * 100:.0f}')

with open(args.target, 'w') as file:
    file.write(template)

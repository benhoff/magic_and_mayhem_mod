"""Prioritize reclaimed bytes while retaining the caller's eligibility checks."""

def order_runs(rows,main_device):
    for row in rows:
        if type(row['allocated_bytes']) is not int or row['allocated_bytes']<0:
            raise ValueError('Run allocation must be a nonnegative integer')
    return sorted(rows,key=lambda row:(row['device']!=main_device,-row['allocated_bytes'],row['path']))

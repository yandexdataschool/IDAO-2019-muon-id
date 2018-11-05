import numpy as np
from sklearn.metrics import make_scorer

# https://stackoverflow.com/questions/21844024/weighted-percentile-using-numpy
def weighted_percentile(a, q, w=None):
    """
    Calculates percentiles associated with a (possibly weighted) array

    Parameters
    ----------
    a : array-like
        The input array from which to calculate percents
    q : array-like
        The percentiles to calculate (0.0 - 100.0)
    w : array-like, optional
        The weights to assign to values of a.  Equal weighting if None
        is specified

    Returns
    -------
    values : np.array
        The values associated with the specified percentiles.  
    """
    # Standardize and sort based on values in a
    q = np.array(q) / 100.0
    if w is None:
        w = np.ones(a.size)
    idx = np.argsort(a)
    a_sort = a[idx]
    w_sort = w[idx]

    # Get the cumulative sum of weights
    ecdf = np.cumsum(w_sort)

    # Find the percentile index positions associated with the percentiles
    p = q * (w.sum() - 1)

    # Find the bounding indices (both low and high)
    idx_low = np.searchsorted(ecdf, p, side='right')
    idx_high = np.searchsorted(ecdf, p + 1, side='right')
    idx_high[idx_high > ecdf.size - 1] = ecdf.size - 1

    # Calculate the weights 
    weights_high = p - np.floor(p)
    weights_low = 1.0 - weights_high

    # Extract the low/high indexes and multiply by the corresponding weights
    x1 = np.take(a_sort, idx_low) * weights_low
    x2 = np.take(a_sort, idx_high) * weights_high

    # Return the average
    return np.add(x1, x2)

def get_rejection_at_efficiency_raw(labels, predictions, weights, quantile):
    signal_indices = (labels >= 1)
    background_indices = ~signal_indices
    if weights is None:
        signal_weights = None
    else:
        signal_weights = weights[signal_indices]
    threshold = weighted_percentile(predictions[signal_indices], 
                                    [quantile], signal_weights)[0]
    rejected_indices = (predictions[background_indices] < threshold)
    if weights is not None:
        rejected_background = weights[background_indices][rejected_indices].sum()
        weights_sum = np.sum(weights[background_indices])
    else:
        rejected_background = rejected_indices.sum()
        weights_sum = np.sum(background_indices)
    return rejected_background, weights_sum         


def get_rejection_at_efficiency(labels, predictions, threshold, sample_weight=None):
    rejected_background, weights_sum = get_rejection_at_efficiency_raw(
        labels, predictions, sample_weight, threshold)
    return rejected_background / weights_sum


rejection10_sklearn = make_scorer(
    get_rejection_at_efficiency, needs_threshold=True, threshold=10)

/* randist/mvgauss.c
 * 
 * Copyright (C) 2016 Yi Jia, Timothée Flutre
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>

/* Generate a random vector from a multivariate Gaussian distribution using the Cholesky
 * decomposition of the variance-covariance matrix following "Computational Statistics"
 * from Gentle (2009), section 7.4.
 *
 *    mu      mean vector (dimension d)
 *    Sigma   Cholesky decomposition of variance-covariance matrix (dimension d x d)
 *    result  output vector (dimension d)
 */
int
gsl_ran_multivariate_gaussian (const gsl_rng * r,
                               const gsl_vector * mu,
                               const gsl_matrix * Sigma_chol,
                               gsl_vector * result)
{
  const size_t M = Sigma_chol->size1;
  const size_t N = Sigma_chol->size2;

  if (M != N)
    {
      GSL_ERROR("multivariate Gaussian requires square matrix", GSL_ENOTSQR);
    }
  else if (mu->size != M)
    {
      GSL_ERROR("multivariate Gaussian requires compatible dimensions for mean vector and variance-covariance matrix", GSL_EBADLEN);
    }
  else
    {
      size_t i;

      for (i = 0; i < N; ++i)
        gsl_vector_set(result, i, gsl_ran_ugaussian(r));

      gsl_blas_dtrmv(CblasLower, CblasNoTrans, CblasNonUnit, Sigma_chol, result);
      gsl_vector_add(result, mu);

      return GSL_SUCCESS;
    }
}

int
gsl_ran_multivariate_gaussian_pdf (const gsl_vector * x,
                                   const gsl_vector * mu,
                                   const gsl_matrix * Sigma_chol,
                                   double * result,
                                   gsl_vector * work)
{
  const size_t M = Sigma_chol->size1;
  const size_t N = Sigma_chol->size2;

  if (M != N)
    {
      GSL_ERROR("multivariate Gaussian requires square matrix", GSL_ENOTSQR);
    }
  else if (x->size != M)
    {
      GSL_ERROR("x vector does not match variance-covariance matrix", GSL_EBADLEN);
    }
  else if (mu->size != M)
    {
      GSL_ERROR("multivariate Gaussian requires compatible dimensions for mean vector and variance-covariance matrix", GSL_EBADLEN);
    }
  else if (work->size != M)
    {
      GSL_ERROR("work vector does not match variance-covariance matrix", GSL_EBADLEN);
    }
  else
    {
      size_t i;
      double quadform;        /* (x - mu)' Sigma^{-1} (x - mu) */
      double logSqrtDetSigma; /* ln [ sqrt(|Sigma|) ] */

      /* compute: work = x - mu */
      for (i = 0; i < M; ++i)
        {
          double xi = gsl_vector_get(x, i);
          double mui = gsl_vector_get(mu, i);
          gsl_vector_set(work, i, xi - mui);
        }

      /* compute: work = L^{-1} * (x - mu) */
      gsl_blas_dtrsv(CblasLower, CblasNoTrans, CblasNonUnit, Sigma_chol, work);

      /* compute: quadform = (x - mu)' Sigma^{-1} (x - mu) */
      gsl_blas_ddot(work, work, &quadform);

      /* compute ln [ sqrt(|Sigma|) ] = sum_i ln L_{ii} */
      logSqrtDetSigma = 0.0;
      for (i = 0; i < M; ++i)
        {
          double Lii = gsl_matrix_get(Sigma_chol, i, i);
          logSqrtDetSigma += log(Lii);
        }

      *result = exp(-0.5*quadform - logSqrtDetSigma - 0.5*M*log(2.0*M_PI));

      return GSL_SUCCESS;
    }
}

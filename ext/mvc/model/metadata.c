
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2012 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_phalcon.h"
#include "phalcon.h"

#include "Zend/zend_operators.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_interfaces.h"

#include "kernel/main.h"
#include "kernel/memory.h"

#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/exception.h"
#include "kernel/file.h"
#include "kernel/operators.h"

/**
 * Phalcon\Mvc\Model\MetaData
 *
 * <p>Because Phalcon\Mvc\Model requires meta-data like field names, data types, primary keys, etc.
 * this component collect them and store for further querying by Phalcon\Model\Base.
 * Phalcon\Mvc\Model\MetaData can also use adapters to store temporarily or permanently the meta-data.</p>
 *
 * <p>A standard Phalcon\Mvc\Model\MetaData can be used to query model attributes:</p>
 *
 * <code>
 *	$metaData = new Phalcon\Mvc\Model\MetaData\Memory();
 *	$attributes = $metaData->getAttributes(new Robots());
 *	print_r($attributes);
 * </code>
 *
 */


/**
 * Phalcon\Mvc\Model\MetaData initializer
 */
PHALCON_INIT_CLASS(Phalcon_Mvc_Model_MetaData){

	PHALCON_REGISTER_CLASS(Phalcon\\Mvc\\Model, MetaData, mvc_model_metadata, phalcon_mvc_model_metadata_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_mvc_model_metadata_ce, SL("_metaData"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_mvc_model_metadata_ce, SL("_columnMap"), ZEND_ACC_PROTECTED TSRMLS_CC);

	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_ATTRIBUTES"), 0 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_PRIMARY_KEY"), 1 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_NON_PRIMARY_KEY"), 2 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_NOT_NULL"), 3 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_DATA_TYPES"), 4 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_DATA_TYPES_NUMERIC"), 5 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_DATE_AT"), 6 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_DATE_IN"), 7 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_IDENTITY_COLUMN"), 8 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_DATA_TYPES_BIND"), 9 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_AUTOMATIC_DEFAULT_INSERT"), 10 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_AUTOMATIC_DEFAULT_UPDATE"), 11 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_COLUMN_MAP"), 0 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_mvc_model_metadata_ce, SL("MODELS_REVERSE_COLUMN_MAP"), 1 TSRMLS_CC);

	return SUCCESS;
}

/**
 * Initialize the metadata for certain table
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param string $key
 * @param string $table
 * @param string $schema
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, _initialize){

	zval *model, *key, *table, *schema, *class_name = NULL, *meta_data = NULL;
	zval *data = NULL, *table_metadata = NULL, *exception_message = NULL;
	zval *connection, *exists, *complete_table = NULL, *columns;
	zval *attributes, *primary_keys, *non_primary_keys;
	zval *numeric_typed, *not_null, *field_types;
	zval *field_bind_types, *automatic_default;
	zval *identity_field = NULL, *column = NULL, *field_name = NULL, *feature = NULL;
	zval *type = NULL, *bind_type = NULL, *column_map = NULL, *ordered_column_map = NULL;
	zval *reversed_column_map = NULL, *user_column_map;
	zval *user_name = NULL, *name = NULL, *model_column_map;
	zval *t0 = NULL;
	HashTable *ah0, *ah1;
	HashPosition hp0, hp1;
	zval **hd;
	char *hash_index;
	uint hash_index_len;
	ulong hash_num;
	int hash_type;
	int eval_int;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzzz", &model, &key, &table, &schema) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(class_name);
	phalcon_get_class(class_name, model TSRMLS_CC);
	if (Z_TYPE_P(key) != IS_NULL) {
		PHALCON_INIT_VAR(meta_data);
		phalcon_read_property(&meta_data, this_ptr, SL("_metaData"), PH_NOISY_CC);
		eval_int = phalcon_array_isset(meta_data, key);
		if (!eval_int) {
			/** 
			 * The meta-data is read from the adapter always
			 */
			PHALCON_INIT_VAR(data);
			PHALCON_CALL_METHOD_PARAMS_1(data, this_ptr, "read", key, PH_NO_CHECK);
			if (Z_TYPE_P(data) != IS_NULL) {
				if (Z_TYPE_P(meta_data) != IS_ARRAY) { 
					PHALCON_INIT_NVAR(meta_data);
					array_init(meta_data);
				}
				phalcon_array_update_zval(&meta_data, key, &data, PH_COPY | PH_SEPARATE TSRMLS_CC);
				phalcon_update_property_zval(this_ptr, SL("_metaData"), meta_data TSRMLS_CC);
			} else {
				/** 
				 * Check if there is a method 'metaData' in the model and retrieve data from it
				 */
				if (phalcon_method_exists_ex(model, SS("metadata") TSRMLS_CC) == SUCCESS) {
					PHALCON_INIT_VAR(table_metadata);
					PHALCON_CALL_METHOD(table_metadata, model, "metadata", PH_NO_CHECK);
					if (Z_TYPE_P(table_metadata) != IS_ARRAY) { 
						PHALCON_INIT_VAR(exception_message);
						PHALCON_CONCAT_SV(exception_message, "Invalid meta-data for model ", class_name);
						PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
						return;
					}
				} else {
					/** 
					 * Check if the mapped table exists on the database
					 */
					PHALCON_INIT_VAR(connection);
					PHALCON_CALL_METHOD(connection, model, "getconnection", PH_NO_CHECK);
	
					PHALCON_INIT_VAR(exists);
					PHALCON_CALL_METHOD_PARAMS_2(exists, connection, "tableexists", table, schema, PH_NO_CHECK);
					if (!zend_is_true(exists)) {
						if (zend_is_true(schema)) {
							PHALCON_INIT_VAR(complete_table);
							PHALCON_CONCAT_VSV(complete_table, schema, "\".\"", table);
						} else {
							PHALCON_CPY_WRT(complete_table, table);
						}
	
						PHALCON_INIT_NVAR(exception_message);
						PHALCON_CONCAT_SVSV(exception_message, "Table \"", complete_table, "\" doesn't exist on database when dumping meta-data for ", class_name);
						PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
						return;
					}
	
					/** 
					 * Try to describe the table
					 */
					PHALCON_INIT_VAR(columns);
					PHALCON_CALL_METHOD_PARAMS_2(columns, connection, "describecolumns", table, schema, PH_NO_CHECK);
					if (!phalcon_fast_count_ev(columns TSRMLS_CC)) {
						if (zend_is_true(schema)) {
							PHALCON_INIT_NVAR(complete_table);
							PHALCON_CONCAT_VSV(complete_table, schema, "\".\"", table);
						} else {
							PHALCON_CPY_WRT(complete_table, table);
						}
	
						PHALCON_INIT_NVAR(exception_message);
						PHALCON_CONCAT_SVSV(exception_message, "Cannot obtain table columns for the mapped source \"", complete_table, "\" used in model ", class_name);
						PHALCON_THROW_EXCEPTION_ZVAL(phalcon_mvc_model_exception_ce, exception_message);
						return;
					}
	
					/** 
					 * Initialize meta-data
					 */
					PHALCON_INIT_VAR(attributes);
					array_init(attributes);
	
					PHALCON_INIT_VAR(primary_keys);
					array_init(primary_keys);
	
					PHALCON_INIT_VAR(non_primary_keys);
					array_init(non_primary_keys);
	
					PHALCON_INIT_VAR(numeric_typed);
					array_init(numeric_typed);
	
					PHALCON_INIT_VAR(not_null);
					array_init(not_null);
	
					PHALCON_INIT_VAR(field_types);
					array_init(field_types);
	
					PHALCON_INIT_VAR(field_bind_types);
					array_init(field_bind_types);
	
					PHALCON_INIT_VAR(automatic_default);
					array_init(automatic_default);
	
					PHALCON_INIT_VAR(identity_field);
					ZVAL_BOOL(identity_field, 0);
	
					if (!phalcon_valid_foreach(columns TSRMLS_CC)) {
						return;
					}
	
					ah0 = Z_ARRVAL_P(columns);
					zend_hash_internal_pointer_reset_ex(ah0, &hp0);
	
					ph_cycle_start_0:
	
						if (zend_hash_get_current_data_ex(ah0, (void**) &hd, &hp0) != SUCCESS) {
							goto ph_cycle_end_0;
						}
	
						PHALCON_GET_FOREACH_VALUE(column);
	
						PHALCON_INIT_NVAR(field_name);
						PHALCON_CALL_METHOD(field_name, column, "getname", PH_NO_CHECK);
						phalcon_array_append(&attributes, field_name, PH_SEPARATE TSRMLS_CC);
	
						PHALCON_INIT_NVAR(feature);
						PHALCON_CALL_METHOD(feature, column, "isprimary", PH_NO_CHECK);
						if (PHALCON_IS_TRUE(feature)) {
							phalcon_array_append(&primary_keys, field_name, PH_SEPARATE TSRMLS_CC);
						} else {
							phalcon_array_append(&non_primary_keys, field_name, PH_SEPARATE TSRMLS_CC);
						}
	
						PHALCON_INIT_NVAR(feature);
						PHALCON_CALL_METHOD(feature, column, "isnumeric", PH_NO_CHECK);
						if (PHALCON_IS_TRUE(feature)) {
							phalcon_array_update_zval_bool(&numeric_typed, field_name, 1, PH_SEPARATE TSRMLS_CC);
						}
	
						PHALCON_INIT_NVAR(feature);
						PHALCON_CALL_METHOD(feature, column, "isnotnull", PH_NO_CHECK);
						if (PHALCON_IS_TRUE(feature)) {
							phalcon_array_append(&not_null, field_name, PH_SEPARATE TSRMLS_CC);
						}
	
						PHALCON_INIT_NVAR(feature);
						PHALCON_CALL_METHOD(feature, column, "isautoincrement", PH_NO_CHECK);
						if (PHALCON_IS_TRUE(feature)) {
							PHALCON_CPY_WRT(identity_field, field_name);
						}
	
						PHALCON_INIT_NVAR(type);
						PHALCON_CALL_METHOD(type, column, "gettype", PH_NO_CHECK);
						phalcon_array_update_zval(&field_types, field_name, &type, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
						PHALCON_INIT_NVAR(bind_type);
						PHALCON_CALL_METHOD(bind_type, column, "getbindtype", PH_NO_CHECK);
						phalcon_array_update_zval(&field_bind_types, field_name, &bind_type, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
						zend_hash_move_forward_ex(ah0, &hp0);
						goto ph_cycle_start_0;
	
					ph_cycle_end_0:
	
					/** 
					 * Create an array using the MODELS_* constants as indexes
					 */
					PHALCON_INIT_NVAR(table_metadata);
					array_init(table_metadata);
					phalcon_array_update_long(&table_metadata, 0, &attributes, PH_COPY | PH_SEPARATE TSRMLS_CC);
					phalcon_array_update_long(&table_metadata, 1, &primary_keys, PH_COPY | PH_SEPARATE TSRMLS_CC);
					phalcon_array_update_long(&table_metadata, 2, &non_primary_keys, PH_COPY | PH_SEPARATE TSRMLS_CC);
					phalcon_array_update_long(&table_metadata, 3, &not_null, PH_COPY | PH_SEPARATE TSRMLS_CC);
					phalcon_array_update_long(&table_metadata, 4, &field_types, PH_COPY | PH_SEPARATE TSRMLS_CC);
					phalcon_array_update_long(&table_metadata, 5, &numeric_typed, PH_COPY | PH_SEPARATE TSRMLS_CC);
					phalcon_array_update_long(&table_metadata, 8, &identity_field, PH_COPY | PH_SEPARATE TSRMLS_CC);
					phalcon_array_update_long(&table_metadata, 9, &field_bind_types, PH_COPY | PH_SEPARATE TSRMLS_CC);
					phalcon_array_update_long(&table_metadata, 10, &automatic_default, PH_COPY | PH_SEPARATE TSRMLS_CC);
					phalcon_array_update_long(&table_metadata, 11, &automatic_default, PH_COPY | PH_SEPARATE TSRMLS_CC);
				}
	
				PHALCON_INIT_VAR(t0);
				phalcon_read_property(&t0, this_ptr, SL("_metaData"), PH_NOISY_CC);
				phalcon_array_update_zval(&t0, key, &table_metadata, PH_COPY TSRMLS_CC);
				phalcon_update_property_zval(this_ptr, SL("_metaData"), t0 TSRMLS_CC);
				PHALCON_CALL_METHOD_PARAMS_2_NORETURN(this_ptr, "write", key, table_metadata, PH_NO_CHECK);
			}
		}
	}
	
	/** 
	 * Check for a column map, store in _columnMap in order and reversed order
	 */
	PHALCON_INIT_NVAR(class_name);
	phalcon_get_class(class_name, model TSRMLS_CC);
	
	PHALCON_INIT_VAR(column_map);
	phalcon_read_property(&column_map, this_ptr, SL("_columnMap"), PH_NOISY_CC);
	eval_int = phalcon_array_isset(column_map, class_name);
	if (!eval_int) {
		if (Z_TYPE_P(column_map) != IS_ARRAY) { 
			PHALCON_INIT_NVAR(column_map);
			array_init(column_map);
		}
	
		PHALCON_INIT_NVAR(data);
		PHALCON_CALL_METHOD_PARAMS_1(data, this_ptr, "read", class_name, PH_NO_CHECK);
		if (Z_TYPE_P(data) != IS_NULL) {
			phalcon_array_update_zval(&column_map, class_name, &data, PH_COPY | PH_SEPARATE TSRMLS_CC);
			phalcon_update_property_zval(this_ptr, SL("_columnMap"), column_map TSRMLS_CC);
		} else {
			PHALCON_INIT_VAR(ordered_column_map);
	
			PHALCON_INIT_VAR(reversed_column_map);
	
			/** 
			 * Check for a columnMap() method on the model
			 */
			if (phalcon_method_exists_ex(model, SS("columnmap") TSRMLS_CC) == SUCCESS) {
				PHALCON_INIT_VAR(user_column_map);
				PHALCON_CALL_METHOD(user_column_map, model, "columnmap", PH_NO_CHECK);
				if (Z_TYPE_P(user_column_map) != IS_ARRAY) { 
					PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "columnMap() not returned an array");
					return;
				}
	
				array_init(reversed_column_map);
				PHALCON_CPY_WRT(ordered_column_map, user_column_map);
	
				if (!phalcon_valid_foreach(user_column_map TSRMLS_CC)) {
					return;
				}
	
				ah1 = Z_ARRVAL_P(user_column_map);
				zend_hash_internal_pointer_reset_ex(ah1, &hp1);
	
				ph_cycle_start_1:
	
					if (zend_hash_get_current_data_ex(ah1, (void**) &hd, &hp1) != SUCCESS) {
						goto ph_cycle_end_1;
					}
	
					PHALCON_GET_FOREACH_KEY(name, ah1, hp1);
					PHALCON_GET_FOREACH_VALUE(user_name);
	
					phalcon_array_update_zval(&reversed_column_map, user_name, &name, PH_COPY | PH_SEPARATE TSRMLS_CC);
	
					zend_hash_move_forward_ex(ah1, &hp1);
					goto ph_cycle_start_1;
	
				ph_cycle_end_1:
				if(0){}
	
			}
	
			PHALCON_INIT_VAR(model_column_map);
			array_init(model_column_map);
			phalcon_array_update_long(&model_column_map, 0, &ordered_column_map, PH_COPY | PH_SEPARATE TSRMLS_CC);
			phalcon_array_update_long(&model_column_map, 1, &reversed_column_map, PH_COPY | PH_SEPARATE TSRMLS_CC);
			phalcon_array_update_zval(&column_map, class_name, &model_column_map, PH_COPY | PH_SEPARATE TSRMLS_CC);
			phalcon_update_property_zval(this_ptr, SL("_columnMap"), column_map TSRMLS_CC);
			PHALCON_CALL_METHOD_PARAMS_2_NORETURN(this_ptr, "write", class_name, model_column_map, PH_NO_CHECK);
		}
	}
	
	PHALCON_MM_RESTORE();
}

/**
 * Reads meta-data for certain model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, readMetaData){

	zval *model, *table, *schema, *key, *meta_data = NULL, *data;
	int eval_int;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	if (Z_TYPE_P(model) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A model instance is required to retrieve the meta-data");
		return;
	}
	
	PHALCON_INIT_VAR(table);
	PHALCON_CALL_METHOD(table, model, "getsource", PH_NO_CHECK);
	
	PHALCON_INIT_VAR(schema);
	PHALCON_CALL_METHOD(schema, model, "getschema", PH_NO_CHECK);
	
	PHALCON_INIT_VAR(key);
	PHALCON_CONCAT_VV(key, schema, table);
	
	PHALCON_INIT_VAR(meta_data);
	phalcon_read_property(&meta_data, this_ptr, SL("_metaData"), PH_NOISY_CC);
	eval_int = phalcon_array_isset(meta_data, key);
	if (!eval_int) {
		PHALCON_CALL_METHOD_PARAMS_4_NORETURN(this_ptr, "_initialize", model, key, table, schema, PH_NO_CHECK);
	
		PHALCON_INIT_NVAR(meta_data);
		phalcon_read_property(&meta_data, this_ptr, SL("_metaData"), PH_NOISY_CC);
	}
	
	PHALCON_INIT_VAR(data);
	phalcon_array_fetch(&data, meta_data, key, PH_NOISY_CC);
	
	RETURN_CCTOR(data);
}

/**
 * Reads meta-data for certain model using a MODEL_* constant
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param int $index
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, readMetaDataIndex){

	zval *model, *index, *table, *schema, *key, *meta_data = NULL;
	zval *attributes;
	zval *r0 = NULL;
	int eval_int;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &model, &index) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	if (Z_TYPE_P(model) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A model instance is required to retrieve the meta-data");
		return;
	}
	if (Z_TYPE_P(index) != IS_LONG) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Index must be a valid integer constant");
		return;
	}
	
	PHALCON_INIT_VAR(table);
	PHALCON_CALL_METHOD(table, model, "getsource", PH_NO_CHECK);
	
	PHALCON_INIT_VAR(schema);
	PHALCON_CALL_METHOD(schema, model, "getschema", PH_NO_CHECK);
	
	PHALCON_INIT_VAR(key);
	PHALCON_CONCAT_VV(key, schema, table);
	
	PHALCON_INIT_VAR(meta_data);
	phalcon_read_property(&meta_data, this_ptr, SL("_metaData"), PH_NOISY_CC);
	eval_int = phalcon_array_isset(meta_data, key);
	if (!eval_int) {
		PHALCON_CALL_METHOD_PARAMS_4_NORETURN(this_ptr, "_initialize", model, key, table, schema, PH_NO_CHECK);
	
		PHALCON_INIT_NVAR(meta_data);
		phalcon_read_property(&meta_data, this_ptr, SL("_metaData"), PH_NOISY_CC);
	}
	
	PHALCON_INIT_VAR(r0);
	phalcon_array_fetch(&r0, meta_data, key, PH_NOISY_CC);
	
	PHALCON_INIT_VAR(attributes);
	phalcon_array_fetch(&attributes, r0, index, PH_NOISY_CC);
	
	RETURN_CCTOR(attributes);
}

/**
 * Writes meta-data for certain model using a MODEL_* constant
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param int $index
 * @param mixed $data
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, writeMetaDataIndex){

	zval *model, *index, *data, *table, *schema, *key, *meta_data = NULL;
	int eval_int;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz", &model, &index, &data) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	if (Z_TYPE_P(model) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A model instance is required to retrieve the meta-data");
		return;
	}
	if (Z_TYPE_P(index) != IS_LONG) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Index must be a valid integer constant");
		return;
	}
	
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		if (Z_TYPE_P(data) != IS_STRING) {
			if (Z_TYPE_P(data) != IS_BOOL) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Invalid data for index");
				return;
			}
		}
	}
	
	PHALCON_INIT_VAR(table);
	PHALCON_CALL_METHOD(table, model, "getsource", PH_NO_CHECK);
	
	PHALCON_INIT_VAR(schema);
	PHALCON_CALL_METHOD(schema, model, "getschema", PH_NO_CHECK);
	
	PHALCON_INIT_VAR(key);
	PHALCON_CONCAT_VV(key, schema, table);
	
	PHALCON_INIT_VAR(meta_data);
	phalcon_read_property(&meta_data, this_ptr, SL("_metaData"), PH_NOISY_CC);
	eval_int = phalcon_array_isset(meta_data, key);
	if (!eval_int) {
		PHALCON_CALL_METHOD_PARAMS_4_NORETURN(this_ptr, "_initialize", model, key, table, schema, PH_NO_CHECK);
	
		PHALCON_INIT_NVAR(meta_data);
		phalcon_read_property(&meta_data, this_ptr, SL("_metaData"), PH_NOISY_CC);
	}
	
	phalcon_array_update_multi_2(&meta_data, key, index, &data, 0 TSRMLS_CC);
	phalcon_update_property_zval(this_ptr, SL("_metaData"), meta_data TSRMLS_CC);
	
	PHALCON_MM_RESTORE();
}

/**
 * Reads the ordered/reversed column map for certain model
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, readColumnMap){

	zval *model, *class_name, *column_map = NULL, *null_value;
	zval *data;
	int eval_int;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	if (Z_TYPE_P(model) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A model instance is required to retrieve the meta-data");
		return;
	}
	
	PHALCON_INIT_VAR(class_name);
	phalcon_get_class(class_name, model TSRMLS_CC);
	
	PHALCON_INIT_VAR(column_map);
	phalcon_read_property(&column_map, this_ptr, SL("_columnMap"), PH_NOISY_CC);
	eval_int = phalcon_array_isset(column_map, class_name);
	if (!eval_int) {
		PHALCON_INIT_VAR(null_value);
		PHALCON_CALL_METHOD_PARAMS_4_NORETURN(this_ptr, "_initialize", model, null_value, null_value, null_value, PH_NO_CHECK);
	
		PHALCON_INIT_NVAR(column_map);
		phalcon_read_property(&column_map, this_ptr, SL("_columnMap"), PH_NOISY_CC);
	}
	
	PHALCON_INIT_VAR(data);
	phalcon_array_fetch(&data, column_map, class_name, PH_NOISY_CC);
	
	RETURN_CCTOR(data);
}

/**
 * Reads column-map information for certain model using a MODEL_* constant
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @param int $index
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, readColumnMapIndex){

	zval *model, *index, *class_name, *column_map = NULL, *null_value;
	zval *attributes;
	zval *r0 = NULL;
	int eval_int;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &model, &index) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	if (Z_TYPE_P(model) != IS_OBJECT) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "A model instance is required to retrieve the meta-data");
		return;
	}
	if (Z_TYPE_P(index) != IS_LONG) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Index must be a valid integer constant");
		return;
	}
	
	PHALCON_INIT_VAR(class_name);
	phalcon_get_class(class_name, model TSRMLS_CC);
	
	PHALCON_INIT_VAR(column_map);
	phalcon_read_property(&column_map, this_ptr, SL("_columnMap"), PH_NOISY_CC);
	eval_int = phalcon_array_isset(column_map, class_name);
	if (!eval_int) {
		PHALCON_INIT_VAR(null_value);
		PHALCON_CALL_METHOD_PARAMS_4_NORETURN(this_ptr, "_initialize", model, null_value, null_value, null_value, PH_NO_CHECK);
	
		PHALCON_INIT_NVAR(column_map);
		phalcon_read_property(&column_map, this_ptr, SL("_columnMap"), PH_NOISY_CC);
	}
	
	PHALCON_INIT_VAR(r0);
	phalcon_array_fetch(&r0, column_map, class_name, PH_NOISY_CC);
	
	PHALCON_INIT_VAR(attributes);
	phalcon_array_fetch(&attributes, r0, index, PH_NOISY_CC);
	
	RETURN_CCTOR(attributes);
}

/**
 * Returns table attributes names (fields)
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return 	array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getAttributes){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 0);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readmetadataindex", model, index, PH_NO_CHECK);
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The meta-data is invalid or is corrupted");
		return;
	}
	
	
	RETURN_CCTOR(data);
}

/**
 * Returns an array of fields which are part of the primary key
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getPrimaryKeyAttributes){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 1);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readmetadataindex", model, index, PH_NO_CHECK);
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The meta-data is invalid or is corrupted");
		return;
	}
	
	
	RETURN_CCTOR(data);
}

/**
 * Returns an arrau of fields which are not part of the primary key
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return 	array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getNonPrimaryKeyAttributes){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 2);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readmetadataindex", model, index, PH_NO_CHECK);
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The meta-data is invalid or is corrupted");
		return;
	}
	
	
	RETURN_CCTOR(data);
}

/**
 * Returns an array of not null attributes
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getNotNullAttributes){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 3);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readmetadataindex", model, index, PH_NO_CHECK);
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The meta-data is invalid or is corrupted");
		return;
	}
	
	
	RETURN_CCTOR(data);
}

/**
 * Returns attributes and their data types
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getDataTypes){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 4);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readmetadataindex", model, index, PH_NO_CHECK);
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The meta-data is invalid or is corrupted");
		return;
	}
	
	
	RETURN_CCTOR(data);
}

/**
 * Returns attributes which types are numerical
 *
 * @param  Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getDataTypesNumeric){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 5);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readmetadataindex", model, index, PH_NO_CHECK);
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The meta-data is invalid or is corrupted");
		return;
	}
	
	
	RETURN_CCTOR(data);
}

/**
 * Returns the name of identity field (if one is present)
 *
 * @param  Phalcon\Mvc\ModelInterface $model
 * @return string
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getIdentityField){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 8);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readmetadataindex", model, index, PH_NO_CHECK);
	
	RETURN_CCTOR(data);
}

/**
 * Returns attributes and their bind data types
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getBindTypes){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 9);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readmetadataindex", model, index, PH_NO_CHECK);
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The meta-data is invalid or is corrupted");
		return;
	}
	
	
	RETURN_CCTOR(data);
}

/**
 * Returns attributes that must be ignored from the INSERT SQL generation
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getAutomaticCreateAttributes){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 10);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readmetadataindex", model, index, PH_NO_CHECK);
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The meta-data is invalid or is corrupted");
		return;
	}
	
	
	RETURN_CCTOR(data);
}

/**
 * Returns attributes that must be ignored from the UPDATE SQL generation
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getAutomaticUpdateAttributes){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 11);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readmetadataindex", model, index, PH_NO_CHECK);
	if (Z_TYPE_P(data) != IS_ARRAY) { 
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The meta-data is invalid or is corrupted");
		return;
	}
	
	
	RETURN_CCTOR(data);
}

/**
 * Set the attributes that must be ignored from the INSERT SQL generation
 *
 * @param  Phalcon\Mvc\ModelInterface $model
 * @param  array $attributes
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, setAutomaticCreateAttributes){

	zval *model, *attributes, *create_index;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &model, &attributes) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(create_index);
	ZVAL_LONG(create_index, 10);
	PHALCON_CALL_METHOD_PARAMS_3_NORETURN(this_ptr, "writemetadataindex", model, create_index, attributes, PH_NO_CHECK);
	
	PHALCON_MM_RESTORE();
}

/**
 * Set the attributes that must be ignored from the UPDATE SQL generation
 *
 * @param  Phalcon\Mvc\ModelInterface $model
 * @param  array $attributes
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, setAutomaticUpdateAttributes){

	zval *model, *attributes, *create_index;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &model, &attributes) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(create_index);
	ZVAL_LONG(create_index, 11);
	PHALCON_CALL_METHOD_PARAMS_3_NORETURN(this_ptr, "writemetadataindex", model, create_index, attributes, PH_NO_CHECK);
	
	PHALCON_MM_RESTORE();
}

/**
 * Returns the column map if any
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getColumnMap){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 0);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readcolumnmapindex", model, index, PH_NO_CHECK);
	if (Z_TYPE_P(data) != IS_NULL) {
		if (Z_TYPE_P(data) != IS_ARRAY) { 
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The meta-data is invalid or is corrupted");
			return;
		}
	}
	
	
	RETURN_CCTOR(data);
}

/**
 * Returns the reverse column map if any
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return array
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, getReverseColumnMap){

	zval *model, *index, *data;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &model) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	PHALCON_INIT_VAR(index);
	ZVAL_LONG(index, 1);
	
	PHALCON_INIT_VAR(data);
	PHALCON_CALL_METHOD_PARAMS_2(data, this_ptr, "readcolumnmapindex", model, index, PH_NO_CHECK);
	if (Z_TYPE_P(data) != IS_NULL) {
		if (Z_TYPE_P(data) != IS_ARRAY) { 
			PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "The meta-data is invalid or is corrupted");
			return;
		}
	}
	
	
	RETURN_CCTOR(data);
}

/**
 * Check if a model has certain attribute
 *
 * @param Phalcon\Mvc\ModelInterface $model
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, hasAttribute){

	zval *model, *attribute, *column_map, *meta_data;
	zval *data_types;
	int eval_int;

	PHALCON_MM_GROW();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &model, &attribute) == FAILURE) {
		PHALCON_MM_RESTORE();
		RETURN_NULL();
	}

	if (Z_TYPE_P(attribute) != IS_STRING) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_mvc_model_exception_ce, "Attribute must be a string");
		return;
	}
	
	PHALCON_INIT_VAR(column_map);
	PHALCON_CALL_METHOD_PARAMS_1(column_map, this_ptr, "getreversecolumnmap", model, PH_NO_CHECK);
	if (Z_TYPE_P(column_map) == IS_ARRAY) { 
		eval_int = phalcon_array_isset(column_map, attribute);
		if (eval_int) {
			PHALCON_MM_RESTORE();
			RETURN_TRUE;
		}
	} else {
		PHALCON_INIT_VAR(meta_data);
		PHALCON_CALL_METHOD_PARAMS_1(meta_data, this_ptr, "readmetadata", model, PH_NO_CHECK);
	
		PHALCON_INIT_VAR(data_types);
		phalcon_array_fetch_long(&data_types, meta_data, 4, PH_NOISY_CC);
		eval_int = phalcon_array_isset(data_types, attribute);
		if (eval_int) {
			PHALCON_MM_RESTORE();
			RETURN_TRUE;
		}
	}
	
	PHALCON_MM_RESTORE();
	RETURN_FALSE;
}

/**
 * Checks if the internal meta-data container is empty
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, isEmpty){

	zval *meta_data, *number, *is_empty;
	zval *t0 = NULL;

	PHALCON_MM_GROW();

	PHALCON_INIT_VAR(meta_data);
	phalcon_read_property(&meta_data, this_ptr, SL("_metaData"), PH_NOISY_CC);
	
	PHALCON_INIT_VAR(number);
	phalcon_fast_count(number, meta_data TSRMLS_CC);
	
	PHALCON_INIT_VAR(t0);
	ZVAL_LONG(t0, 0);
	
	PHALCON_INIT_VAR(is_empty);
	is_equal_function(is_empty, number, t0 TSRMLS_CC);
	
	RETURN_NCTOR(is_empty);
}

/**
 * Resets internal meta-data in order to regenerate it
 */
PHP_METHOD(Phalcon_Mvc_Model_MetaData, reset){

	zval *empty_array;

	PHALCON_MM_GROW();

	PHALCON_INIT_VAR(empty_array);
	array_init(empty_array);
	phalcon_update_property_zval(this_ptr, SL("_metaData"), empty_array TSRMLS_CC);
	
	PHALCON_MM_RESTORE();
}


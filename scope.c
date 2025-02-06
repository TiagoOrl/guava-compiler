#include "helpers/vector.h"
#include "compiler.h"
#include "memory.h"
#include <stdlib.h>
#include <assert.h>



struct scope* scope_alloc() 
{
    struct scope* scope = calloc(1, sizeof(struct scope));
    scope->entities = vector_create(sizeof(void*));
    vector_set_peek_pointer_end(scope->entities);
    vector_set_flag(scope->entities, VECTOR_FLAG_PEEK_DECREMENT);
    return scope;
}


void scope_dealloc(struct scope* scope)
{
    // do nothing for now
}


struct scope* scope_create_root(struct compile_process* process)
{
    assert(!process->scope.root);
    assert(!process->scope.current);

    struct scope* root_scope = scope_alloc();
    process->scope.root = root_scope;
    process->scope.current = root_scope;

    return root_scope;
}


void scope_free_root(struct compile_process* process)
{
    scope_dealloc(process->scope.root);
    process->scope.root = NULL;
    process->scope.current = NULL;
}


struct scope* scope_new(struct compile_process* process, int flags)
{
    assert(process->scope.root);
    assert(process->scope.current);

    struct scope* new_scope = scope_alloc();
    new_scope->flags = flags;
    new_scope->parent = process->scope.current;
    process->scope.current = new_scope;

    return new_scope;
}


void scope_iteration_start(struct scope* scope) 
{
    vector_set_peek_pointer(scope->entities, 0);

    if (scope->entities->flags & VECTOR_FLAG_PEEK_DECREMENT)
    {
        vector_set_peek_pointer_end(scope->entities);
    }
}


void scope_iteration_end(struct scope* scope) 
{

}


void* scope_iterate_back(struct scope* scope)
{
    if(vector_count(scope->entities) == 0)
        return NULL;

    return vector_peek_ptr(scope->entities);
}


void* scope_last_entity_of_scope(struct scope* scope) 
{
    if(vector_count(scope->entities) == 0)
        return NULL;

    return vector_back_ptr(scope->entities);
}